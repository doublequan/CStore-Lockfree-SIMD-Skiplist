export void index_compare_ispc(uniform int indexes[], uniform int key, uniform int max_size, 
                               uniform int index_size, uniform unsigned int8 result[])
{
    foreach (i = 0 ... max_size) {
        if(programIndex <index_size) {
            result[i] = (key < indexes[i]?1:0);
        }else{
            result[i] = 1;
        }
    }
}
