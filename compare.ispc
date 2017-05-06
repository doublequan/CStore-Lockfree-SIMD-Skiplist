export void index_compare_ispc(uniform std::string[] indexes, uniform std::string key,
                            uniform int index_size, uniform std::string result)
{
    foreach (i = 0 ... index_size) {
        result[i] = key<indexes[i]+'0';
    }
}