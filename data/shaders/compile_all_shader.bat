@echo off
for /r %%i in (*.frag *.vert) do (
	For %%A in (%%i) do (
		Set Folder=%%~dpA
		Set Name=%%~nxA
	)
	
	echo.Name is %Name%
	
	IF %Name% == screen_quad_vert_recon.vert (
		glslc %%i -o screen_quad.vert.spv
		glslc %%i -DENABLE_VERTEX_RECONSTRUCTION -o screen_quad_vert_recon.vert.spv
		glslc %%i -DENABLE_WORLD_SPACE_VIEW_VEC -o screen_quad_ws_view.vert.spv
		glslc %%i -DENABLE_VERTEX_RECONSTRUCTION -DENABLE_WORLD_SPACE_VIEW_VEC -o screen_quad_vert_recon_ws_view.vert.spv
	) ELSE ( 
		glslc %%i -o %%i.spv
	)
)