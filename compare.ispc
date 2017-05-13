export inline uniform int index_compare_ispc_sum(uniform int indexes[], uniform int key, uniform int max_size,
                               uniform int index_size)
{
 int sum = 0;
    foreach (i = 0 ... max_size) {
        if(i < index_size) {
            sum += (key < indexes[i]?1:0);
        }else{
            sum += 1;
        }
    }
    return reduce_add(sum);
}

export inline void index_compare_ispc(uniform int indexes[], uniform int key, uniform int max_size,
                               uniform int index_size, uniform unsigned int8 result[])
{
    foreach (i = 0 ... max_size) {
        if(i <index_size) {
            result[i] = (key < indexes[i]?1:0);
        }else{
            result[i] = 1;
        }
    }
}