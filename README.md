# Comp304-Project3

- Çisem Özden 69707
- Gülbarin Maçin 69163

Part 1:

- In this part we made basic mapping between backing memory and main memory because that physical page an logical page have sama size.

- search_tlb and add_t_tlb were implemented. 

Part 2:

- FIFO: 

-- In the fifo we made basic searching with first part1 implementation and replacing. The first allocated frame was replaced the old one. 

- LRU:

-- In the LRU part we used node for replacement. Data, age and the next node is included on the node. We traversed the list, find the node which have biggest age value. Then the LRU function returns to the data of the this node. Then this function used in the main for LRU replecement.
