# fix this for your own path to directx
FIND_PATH(DIRECTX_BASECLASSES_DIR NAMES amfilter.h PATHS
	  			$ENV{HOME}/directx 
				$ENV{HOME}/directx/BaseClasses
				/directx 
				/directx/BaseClasses
				c:/directx
				c:/directx/BaseClasses)
FIND_PATH(DIRECTX_INCLUDE_DIR NAMES dsound.h PATHS
				$ENV{HOME}/directx 
				$ENV{HOME}/directx/Include 
				/directx 
				/directx/Include 
				c:/directx)



find_path(DIRECTX_INCLUDE_DIR dxdiag.h /directx/Include)

find_path(DIRECTX_BASECLASSES_DIR dxdiag.h /directx/Include)
