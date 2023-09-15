import os
from PIL import Image

fileList = []

for (dirpath, dirnames, filenames) in os.walk("."):
	for file in filenames:
		if (file.endswith('.png') or file.endswith('.jpg')):
			path = os.path.join(dirpath, file)
			with Image.open(path) as img:
				width, height = img.size
			path = path.replace('\\', '/')
			path = path.replace('./', '')
			fileList.append([path, width, height])

handle = open("image_registry.json", "w")
handle.write("""{
	"paths": [
""")
for i, fileInfo in enumerate(fileList):
	file = fileInfo[0]
	width = fileInfo[1]
	height = fileInfo[2]
	id = os.path.splitext(file)[0]
	comma = '' if (i == len(fileList) - 1) else ','
	handle.write('\t\t{ "id": "%s", "path": "%s", "width": %d, "height": %d }%s\n' % (id,  file, width, height, comma))

handle.write("""\t]
}""")

handle.close()