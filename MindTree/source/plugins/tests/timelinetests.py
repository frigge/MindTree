import MT

def testTimeline():
   timelinenode = MT.createNode("Values.Frame")
   floatnode = MT.createNode("Values.Float Value")

   floatnode.insockets[0].connected = timelinenode.outsockets[0]

   cache = MT.cache.DataCache(floatnode.outsockets[0])
   print("initial frame value as {}: {}".format(cache.type, cache.getOutput()))
   
   MT.timeline.setFrame(44)
   MT.cache.DataCache.invalidate(timelinenode)
   cache = MT.cache.DataCache(floatnode.outsockets[0])
   print("new cached frame value as %s: %d" % (cache.type, cache.getOutput()))
   print("new frame value: %d" % (MT.timeline.frame()))

   return int(cache.getOutput()) == 44
