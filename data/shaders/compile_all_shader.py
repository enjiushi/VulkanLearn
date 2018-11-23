from pathlib import Path
import os

def compile_shader(abs_path, ext):

	path_list = Path(abs_path).glob('**/*.' + ext)

	for path in path_list:
		path_in_string = str(path)
		_path, _file = os.path.split(path_in_string)
		
		if _file == 'screen_quad.vert':
			cmd = 'glslc ' + path_in_string + ' -DENABLE_WS_POS_RECONSTRUCTION -o ' + _path + '/screen_quad_vert_recon.vert.spv'
			print(cmd)
			os.system(cmd)
			
			cmd = 'glslc ' + path_in_string + ' -DENABLE_WS_VIEW_RAY -o ' + _path + '/screen_quad_ws_view_ray.vert.spv'
			print(cmd)
			os.system(cmd)
			
			cmd = 'glslc ' + path_in_string + ' -DENABLE_WS_POS_RECONSTRUCTION -DENABLE_WS_VIEW_RAY -o ' + _path + '/screen_quad_vert_recon_ws_view_ray.vert.spv'
			print(cmd)
			os.system(cmd)

		cmd = 'glslc ' + path_in_string + ' -o ' + path_in_string + '.spv'
		print(cmd)
		os.system(cmd)
	
cur_path = os.path.dirname(os.path.abspath(__file__))
	
compile_shader(cur_path, 'vert')
compile_shader(cur_path, 'frag')