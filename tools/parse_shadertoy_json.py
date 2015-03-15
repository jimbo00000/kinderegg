# parse_shadertoy_json.py

from __future__ import print_function
import json
import os
import sys
import shutil
import requests
from PIL import Image

def dumpReadmeFile(info, id):
	readmeFileOut = os.path.join(id, 'README.txt')
	with open(readmeFileOut,'w') as outStream:
		print("Title: " + info['name'], file=outStream)
		print("Author: " + info['username'], file=outStream)
		print('',file=outStream)
		print("Tags: " + ', '.join(info['tags']), file=outStream)
		print('',file=outStream)
		print("Description: " + info['description'], file=outStream)
		print('',file=outStream)
		print('Generated from https://www.shadertoy.com/view/{0} by kinderegg.'.format(info['id']),file=outStream)


textureHeader = """/*
 * Generated by parse_shadertoy_json.py
 */
"""
def dumpTextureHeader(renderpass):
	readmeFileOut = 'g_textures.h'
	texDir = os.path.join('..', 'textures')
	with open(readmeFileOut,'w') as outStream:
		print(textureHeader, file=outStream)
		pass_id = 0
		for r in renderpass:
			# Pull out textures
			tex_id = 0
			for t in r['inputs']:
				texfile = os.path.basename(t['src'])
				print("    tex" + str(t['channel']) + ": " + texfile, end='')
				img = Image.open(os.path.join(texDir,texfile))
				px = img.load()

				arrayname = 'tex' + str(pass_id) + str(tex_id)
				vardecl = 'int {0} = {1};'
				print(vardecl.format(arrayname+'w', img.size[0]), file=outStream)
				print(vardecl.format(arrayname+'h', img.size[1]), file=outStream)

				arrdecl = 'char {0}[] = '
				print(arrdecl.format(arrayname), file=outStream)
				print('{', file=outStream)
				for j in range(img.size[1]):
					for i in range(img.size[0]):
						p = px[i,j]
						if isinstance(p,int):
							print(" " + str(p) + ",", end='', file=outStream)
						elif isinstance(p,tuple):
							for x in p:
								print(" " + str(x) + ",", end='', file=outStream)
				print('}', file=outStream)
				print('', file=outStream)

				tex_id += 1
			pass_id += 1


def dumpShaderFiles(renderpass):
	"""Save shader source to ../shaders/ for hardcoding by CMake."""
	shaderDir = os.path.join('..', 'shaders')
	for r in renderpass:
		shfile = r['type'] + ".frag"
		src = r['code']
		# TODO some retroactive refactoring
		#src = src.replace("main", "mainImage")
		#src = src.replace("gl_FragColor", "glFragColor")
		with open(os.path.join(shaderDir, shfile),'w') as outStream:
			print(src, file=outStream)
		print(shfile + ": " + str(len(src)) + " bytes written.")


def parseJson(j):
	#print(json.dumps(j,indent=1))
	info = j['Shader']['info']
	dumpReadmeFile(info)
	renderpass = j['Shader']['renderpass']
	#dumpTextureHeader(renderpass)
	dumpShaderFiles(renderpass)


def queryShadertoy(id):
	"""Send a request to Shaderoy.com for the given shadertoy id.
	Store your API key in the filename below.
	"""
	apikey = "xxxxxx"
	with open('apikey.txt','r') as keystr:
		apikey = keystr.read()
	req = 'https://www.shadertoy.com/api/v1/shaders/{0}?key={1}'
	req = req.format(id, apikey)
	r = requests.get(req)
	print(r)
	j = r.json()
	if 'Error' in j:
		print(j['Error'])
	else:
		print('Success')
		os.mkdir(id)
		info = j['Shader']['info']
		dumpReadmeFile(info, id)
		renderpass = j['Shader']['renderpass']
		dumpShaderFiles(renderpass)


def invokeBuild():
	"""Invoke CMake which in turn invokes designated compiler to build the executable."""
	cmakepath = '"C:/Program Files (x86)/CMake/bin/cmake"'
	slnpath = '../build'
	os.chdir(slnpath)
	cmds = [
		cmakepath + ' --build . --config Release',
		'dir',]
	for c in cmds:
		print(c)
		os.system(c)
	sdlpath = 'C:/lib/SDL2-2.0.3'
	sdllibpath = 'lib/x86'
	sdldllname = 'SDL2.dll'
	shutil.copyfile(
		os.path.join(sdlpath, sdllibpath, sdldllname),
		os.path.join('.', sdldllname))


#
# Main: enter here
#
def main(argv=None):
	# https://www.shadertoy.com/api/v1/shaders/query/string?key=appkey
	# ldXXDj
	# 4dfXWj
	# lssXWS
	# XdfXWS
	# lsfXDl
	# 4df3D8
	# 4dl3zn
	# MdB3Rc
	# 4tfGRM
	# lts3zn
	if len(sys.argv) <= 1:
		print("Usage: requires one argument(shadertoy id)")
		quit()
	#queryShadertoy(sys.argv[1])
	invokeBuild()


if __name__ == "__main__":
	sys.exit(main())
