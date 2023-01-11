# Comp304-Project3

- Çisem Özden 69707
- Gülbarin Maçin 69163

Part 1:

- In this part we made basic mapping between backing memory and main memory due to the fact that physical page and logical page have the same size.

- search_tlb and add_t_tlb functions were implemented. 

Part 2:

- FIFO: 

-- In the fifo replacement, we made basic searching like we've done in part1 and also replacing. Here the procedure is as follows: next frame is replaced with the one that was used first in the case of all frames are used by some other processes. First come first serve concept is implemented in a circular way. 

- LRU:

-- In the LRU part we used a struct what we called LRU_node for the LRU replacement. Using these nodes we implemented a linked-list structure to keep track of the replacement. Data, age value and the next node fields are used to maintain the implementation. We've done the implementation such that the node with the minimum age value denotes the least recently used page. Therefore, we traversed the list, find the node which have the largest age value. Then this node is popped from the linked-list and the policy continues as such.
