import praw
import paho.mqtt.publish as publish

# Change the user agent string to be unique
r = praw.Reddit(user_agent='iot.TIL.text2speech 1.0 (by /u/jasonfbenton)')

#Pick your subreddit
subreddit=r.get_subreddit('TodayILearned')

# Choose which posts you want (hot, top, new, etc.)
for submission in subreddit.get_hot(limit=10):
	print submission.title
	payload=submission.title
	# Publish to MQTT broker under /TIL topic
	publish.single("/TIL", payload, hostname="atliot.com")


