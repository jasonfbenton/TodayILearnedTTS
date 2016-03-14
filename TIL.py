import praw
import paho.mqtt.publish as publish

r = praw.Reddit(user_agent='iot.TIL.text2speech 1.0 (by /u/jasonfbenton)')

subreddit=r.get_subreddit('TodayILearned')

for submission in subreddit.get_hot(limit=10):
	print submission.title
	payload=submission.title
	publish.single("/TIL", payload, hostname="atliot.com")


