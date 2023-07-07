import os
import math

# Improting Image class from PIL module 
from PIL import Image 

ThumbWidth = 180.0
ThumbHeight = 100.0

def GenThumb(file):
	im = Image.open(file) 
	width, height = im.size
	scaleW = ThumbWidth / width;
	scaleH = ThumbHeight / height;
	
	scale = scaleW
	if height * scaleW < ThumbHeight:
		scale = scaleH
		
	TargetWidth = int(math.ceil(width * scale))
	TargetHeight = int(math.ceil(height * scale))
	
	print ("%s: %d, %d" % (file, TargetWidth, TargetHeight))
	
	im = im.resize((TargetWidth, TargetHeight), Image.LANCZOS)
	im = im.crop((0, 0, ThumbWidth, ThumbHeight))
	filename = file.replace('.jpg', '').replace('.png', '')
	im.save('thumb/%s_t.jpg' % filename, quality=95)
	
	
for r, d, f in os.walk('.'):
	if r == '.':
		for file in f:
			if file.endswith('.png') or file.endswith('.jpg'):
				GenThumb(file)
			


