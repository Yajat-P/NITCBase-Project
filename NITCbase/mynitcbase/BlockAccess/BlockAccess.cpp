#include "BlockAccess.h"
#include<stdlib.h>

#include <cstring>



RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    RecId *searchIndex = (RecId*) malloc (sizeof(RecId));
    RelCacheTable::getSearchIndex(relId, searchIndex);

    RelCatEntry relCatBuf;
    // let block and slot denote the record id of the record being currently checked
    


    // if the current search index record is invalid(i.e. both block and slot = -1)
    if (searchIndex->block == -1 && searchIndex->slot == -1)
    {
        // (no hits from previous search; search should start from the
        // first record itself)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCacheTable::getRelCatEntry(relId, &relCatBuf);
        searchIndex->block =  relCatBuf.firstBlk;
        searchIndex->slot = 0;


        // block = first record block of the relation
        // slot = 0
    }
    else
    {
        // (there is a hit from previous search; search should start from
        // the record next to the search index record)


        searchIndex->block = searchIndex->block;
        searchIndex->slot = searchIndex->slot+1;

        // block = search index's block
        // slot = search index's slot + 1
    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    while (searchIndex->block != -1)
    {
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */
        RecBuffer cblock(searchIndex->block);
        Attribute record[relCatBuf.numAttrs];
        struct HeadInfo blockHeader;
        


        cblock.getRecord(record, searchIndex->slot);
        cblock.getHeader(&blockHeader);
        unsigned char slotMap [blockHeader.numSlots];
        cblock.getSlotMap(slotMap);

        // get the record with id (block, slot) using RecBuffer::getRecord()
        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function

        // If slot >= the number of slots per block(i.e. no more slots in this block)
        if(searchIndex->slot>=blockHeader.numSlots)
        {
            // update block = right block of block
            searchIndex->block = blockHeader.rblock;
            searchIndex->slot = 0;
            
            // update slot = 0
            continue;  // continue to the beginning of this while loop
        }

        // if slot is free skip the loop
        // (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
        if(slotMap[searchIndex->slot]==SLOT_UNOCCUPIED)
        {
            searchIndex->slot++;
        }

        // compare record's attribute value to the the given attrVal as below:
        /*
            firstly get the attribute offset for the attrName attribute
            from the attribute cache entry of the relation using
            AttrCacheTable::getAttrCatEntry()
        */
        /* use the attribute offset to get the value of the attribute from
           current record */
        AttrCatEntry attrCatBuf;
        AttrCacheTable::getAttrCatEntry(relId,attrName, &attrCatBuf);
        int offset = attrCatBuf.offset;
        Attribute attrvalue = record[offset];

         
        

        
        
        










        int cmpVal=compareAttrs(attrvalue, attrVal, attrCatBuf.attrType);
          // will store the difference between the attributes
        // set cmpVal using compareAttrs()

        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */
        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            /*
            set the search index in the relation cache as
            the record id of the record that satisfies the given condition
            (use RelCacheTable::setSearchIndex function)
            */
            RelCacheTable::setSearchIndex(relId, searchIndex);

            return RecId{searchIndex->block, searchIndex->slot};
        }

        searchIndex->slot++;
    }

    // no record in the relation with Id relid satisfies the given condition
    return RecId{-1, -1};
}

