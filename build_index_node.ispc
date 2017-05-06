export void build_index_node(uniform IndexNode*[] leveled_nodes, uniform int[] level_index_count,
                                uniform int vector_size, uniform int max_height, uniform int current_node_height,
                                uniform std::string key, uniform std::string[] routing_keys,
                                uniform std::string default_routing_key)
{
    foreach (i = 0 ... max_height) {
        if(i < current_node_height){
            if(level_index_count[i] >= vector_size){
                IndexNode* node = new IndexNode();
                leveled_nodes[i]->next = node;
                level_index_count[i] = 0;
                routing_keys[i] = default_routing_key;
            }
            leveled_nodes[i]->indexes[level_index_count[i]++] = key;
        }
    }
}

export void set_node_pointer(uniform IndexNode*[] leveled_nodes, uniform std::string[] routing_keys,
                                uniform int[] level_index_count, uniform int max_height,
                                uniform int current_node_height){
    foreach (i = 0 ... max_height) {
        if(i < current_node_height){
            routing_keys[i][level_index_count[i]-1] = '0';
            if (i != 0)
                leveled_nodes[i]->routing_table[routing_keys[i]] = leveled_nodes[i-1];
        }
    }
}