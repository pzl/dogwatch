import os, sys
import datetime
import mandrill
import config

client = mandrill.Mandrill(config.KEY)
barks = sys.argv[1]


content = "As of %(time)s, %(dog)s has barked %(barks)s times" % {
					'time':datetime.datetime.now().strftime("%-I:%M %p, %a %b %d"),
					'dog': config.DOG_NAME,
					'barks': barks,
				}

print('sending message: "%s"' % (content,))

message = {
	'auto_html': False,
	'auto_text': False,
	'important': True,
	'inline_css': False,
	'merge': False,
	'preserve_recipients': True,
	'track_clicks': False,
	'track_opens': True,
	'tracking_domain': None,
	'url_strip_qs': False,
	'from_email': config.FROM['address'],
	'from_name': config.FROM['name'],
	'to': [{
		'email': config.TO['address'],
		'name': config.TO['name'],
		'type': 'to'
	}],
	'headers': {'Reply-To': config.FROM['reply_to']},
	'subject': 'Bark Alert',
	'text': content,
	'html': '<p>%s</p>' % (content,),
}
try:
	result = client.messages.send(message=message, async=False)
	print(result)
except mandrill.Error as e:
	print("A mandrill error occurred: %s - %s" % (e.__class__,e))