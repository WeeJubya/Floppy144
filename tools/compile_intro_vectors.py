from pptx import Presentation
from lxml import etree
from zipfile import ZipFile
from pathlib import Path
import argparse
parser=argparse.ArgumentParser(description="Flatten authored Intro.pptx vector layouts; development only (requires python-pptx).")
parser.add_argument("source")
parser.add_argument("--output",default="game/src/floppy144_intro_vectors.generated.inc")
args=parser.parse_args()
p=Presentation(args.source)
z=ZipFile(args.source)
theme=etree.fromstring(z.read('ppt/theme/theme1.xml'))
ns={'a':'http://schemas.openxmlformats.org/drawingml/2006/main'}
colors={}
for e in theme.find('.//a:clrScheme',ns):
 c=e[0];v=c.get('val') if c.tag.endswith('srgbClr') else c.get('lastClr');colors[etree.QName(e).localname]=v
colors.update(bg1=colors['lt1'],tx1=colors['dk1'],bg2=colors['lt2'],tx2=colors['dk2'])
def color(s):
 es=s._element.xpath('./p:spPr/a:solidFill/*')
 if not es:return None
 e=es[0];v=e.get('val');v=colors.get(v,v)
 c=[int(v[j:j+2],16) for j in (0,2,4)]
 for t in e:
  val=int(t.get('val'))/100000
  if t.tag.endswith('lumMod'):c=[round(x*val) for x in c]
  elif t.tag.endswith('lumOff'):c=[min(255,round(x+255*val)) for x in c]
 return (c[0]<<16)|(c[1]<<8)|c[2]
def flatten(shapes,m=(1,1,0,0)):
 sx,sy,tx,ty=m
 for s in shapes:
  if s.shape_type==6:
   x=s._element.xpath('./p:grpSpPr/a:xfrm')[0]
   off,ext,chof,chex=x
   ax=int(ext.get('cx'))/int(chex.get('cx'));ay=int(ext.get('cy'))/int(chex.get('cy'))
   nm=(sx*ax,sy*ay,tx+sx*(int(off.get('x'))-ax*int(chof.get('x'))),ty+sy*(int(off.get('y'))-ay*int(chof.get('y'))))
   yield from flatten(s.shapes,nm)
  else:
   c=color(s)
   kind=1
   geom=s._element.xpath('.//a:prstGeom/@prst')
   if geom and geom[0]=='snip1Rect':kind=2
   if geom and geom[0]=='upArrow':kind=3
   if s.has_text_frame and s.text.strip():kind=4;c=0x202020
   if c is None:continue
   vals=[round((tx+sx*s.left)*640/p.slide_width),round((ty+sy*s.top)*360/p.slide_height),round(sx*s.width*640/p.slide_width),round(sy*s.height*360/p.slide_height)]
   if vals[2]<=0 or vals[3]<=0:continue
   # Restrict huge off-screen rectangles to safe signed integer coordinates.
   yield (*vals,c,kind,s.shape_id)
a=[]
for i in range(3):a.append(list(flatten(p.slides[i].shapes)))
assert [row[-1] for row in a[0]]==[row[-1] for row in a[1]], 'Discovery/reveal vector correspondence changed'
# BF20: Seat the discovery cabinet and grey foreground panel against the
# rear wall's bottom edge, with a one-pixel floor overlap. This is an export
# adjustment: the original PowerPoint vectors and all later scenes stay intact.
assert [a[0][i][-1] for i in (1, 2, 13)] == [10, 28, 13], 'Discovery anchors changed'
wall_bottom = a[0][1][1] + a[0][1][3]
disk_drop = wall_bottom + 1 - (a[0][2][1] + a[0][2][3])
panel_drop = wall_bottom + 1 - (a[0][13][1] + a[0][13][3])
aligned = []
for index, (x, y, w, h, colour, kind, shape_id) in enumerate(a[0]):
    if 2 <= index <= 12:
        y += disk_drop
    elif index == 13:
        y += panel_drop
    aligned.append((x, y, w, h, colour, kind, shape_id))
a[0] = aligned
q=Path(args.output)
with q.open('w') as f:
 f.write('/* Procedural vector geometry flattened from authored Intro.pptx.\n * No stored image/media. Coordinates target the logical 640x360 surface. */\n')
 for i,rows in enumerate(a):
  f.write('static const IntroVector g_intro_scene'+str(i+1)+'[]={\n')
  for x,y,w,h,c,k,id in rows:f.write('    {%d,%d,%d,%d,0x%06xU,%d}, /* shape %d */\n'%(x,y,w,h,c,k,id))
  f.write('};\n')
print([(i+1,len(rows)) for i,rows in enumerate(a)])
print('screen3',a[2][:6])
