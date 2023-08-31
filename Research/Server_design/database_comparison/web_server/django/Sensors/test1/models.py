from django.db import models

# Create your models here.
class SensorData(models.Model):
	type = models.CharField(max_length=30)
	value = models.CharField(max_length=50)
	source = models.CharField(max_length=50)
	data_date = models.DateField(auto_now_add=True, blank=True)
	data_time = models.TimeField(auto_now_add=True, blank=True)

	def __unicode__(self):
		return u'%s: %s, %s' % (self.source, self.type, self.value)
