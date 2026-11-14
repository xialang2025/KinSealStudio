"""Independently integrate straight path segments over a 256-unit cell lattice."""
from collections import defaultdict
import struct
import numpy as np

def coverage(serialized):
 count=struct.unpack_from('>I',serialized)[0]
 elements=[struct.unpack_from('>Idd',serialized,4+i*20) for i in range(count)]
 rule=struct.unpack_from('>II',serialized,4+count*20)[1]
 cells=defaultdict(lambda:[0,0]);edges=[];first=previous=None
 for tag,x,y in elements:
  assert tag in (0,1),'Straight-edge candidate refuses curves'
  point=(int(x*256),int(y*256))
  if tag==0:
   if previous is not None and previous!=first:edges.append((previous,first))
   first=previous=point
  else:edges.append((previous,point));previous=point
 if previous!=first:edges.append((previous,first))
 def cuts(a,b,axis):
  if a[axis]==b[axis]:return [a,b]
  lo,hi=sorted((a[axis],b[axis]));boundaries=list(range((lo//256+1)*256,hi,256))
  if b[axis]<a[axis]:boundaries.reverse()
  other=1-axis
  return [a]+[tuple(boundary if i==axis else a[other]+((b[other]-a[other])*abs(boundary-a[axis]))//abs(b[axis]-a[axis]) for i in (0,1)) for boundary in boundaries]+[b]
 for a,b in edges:
  if a[1]==b[1]:continue
  rows=cuts(a,b,1)
  for start,end in zip(rows,rows[1:]):
   y=(start[1]+end[1])//512
   if not 0<=y<480:continue
   segments=cuts(start,end,0)
   for p,q in zip(segments,segments[1:]):
    x=(p[0]+q[0])//512;dy=q[1]-p[1]
    cells[y,x][0]+=dy;cells[y,x][1]+=((p[0]-x*256)+(q[0]-x*256))*dy
 mask=np.zeros((480,800),dtype=np.uint8)
 for y in range(480):
  running=0;last=0
  for x,(cover,area) in sorted((x,v) for (row,x),v in cells.items() if row==y):
   gap=abs(running)
   if not rule:gap=gap%512;gap=512-gap if gap>256 else gap
   mask[y,max(0,last):min(800,x)]=min(255,gap)
   running+=cover;alpha=abs((running*512-area)//512)
   if not rule:alpha=alpha%512;alpha=512-alpha if alpha>256 else alpha
   if 0<=x<800:mask[y,x]=min(255,alpha)
   last=x+1
 return mask
