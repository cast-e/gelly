function(find_hlsl_files directory output_var)
    file(GLOB_RECURSE files "${directory}/*.hlsl")
    foreach (file ${files})
        get_filename_component(file_name ${file} NAME_WLE)
        if (${file_name} MATCHES ".+embed")
            continue()
        endif ()
        list(APPEND file_names ${file_name})
    endforeach ()
    set(${output_var} ${file_names} PARENT_SCOPE)
endfunction()

function(find_hlsli_files directory output_var)
    file(GLOB_RECURSE files "${directory}/*.hlsli")
    foreach (file ${files})
        get_filename_component(file_name ${file} NAME_WE)
        list(APPEND file_names ${file_name})
    endforeach ()
    set(${output_var} ${file_names} PARENT_SCOPE)
endfunction()