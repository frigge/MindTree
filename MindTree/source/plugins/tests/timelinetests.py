import MT

def testTimeline():
   timelinenode = MT.createNode("Values.Frame")
   floatnode = MT.createNode("Values.Float Value")

   floatnode.insockets[0].connected = timelinenode.outsockets[0]

   cache = MT.cache.DataCache(floatnode.outsockets[0])
   print("initial frame value as %s: %d" % (cache.type, cache.getOutput()))
   
   MT.timeline.setFrame(44)
   cache = MT.cache.DataCache(floatnode.outsockets[0])
   print("new frame value as %s: %d" % (cache.type, cache.getOutput()))

   return int(cache.getOutput()) == 44
