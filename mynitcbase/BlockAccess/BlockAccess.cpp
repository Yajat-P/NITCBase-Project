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

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal,newName);
    // search the relation catalog for an entry with "RelName" = newRelationName
    char attributename[8];
    strcpy(attributename, "RelName");
    RecId retvaluenew = linearSearch(RELCAT_RELID, attributename, newRelationName, EQ);
    if(!(retvaluenew.block ==-1 && retvaluenew.slot==-1))
    {
        return E_RELEXIST;
    }

    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;


    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName; 
    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal, oldName);
    // search the relation catalog for an entry with "RelName" = oldRelationName
    RecId retvalueold = linearSearch(RELCAT_RELID, attributename, oldRelationName, EQ);

    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    if(retvalueold.block ==-1 && retvalueold.slot==-1)
    {
        return E_RELNOTEXIST;
    }

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
   RecBuffer block(retvalueold.block);
   Attribute record[RELCAT_NO_ATTRS];
   block.getRecord(record, retvalueold.slot); 

 /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal, newName);
    block.setRecord(record, retvalueold.slot);
    
    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord
    

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    
    RecId AttrCatRecId;
    for(int i=0; i<record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;i++)
    {
        AttrCatRecId =  linearSearch(ATTRCAT_RELID, attributename, oldRelationName, EQ);
        RecBuffer attrCatBlock(AttrCatRecId.block);
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        
        attrCatBlock.getRecord(attrCatRecord, AttrCatRecId.slot);
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
        int ret = attrCatBlock.setRecord(attrCatRecord, AttrCatRecId.slot);
        if(ret!=SUCCESS)
        {
            return ret;

        }
    



    }

    return SUCCESS;
}
int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    RecBuffer AttrCatBlock(ATTRCAT_BLOCK);
    


    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal, relName);
    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    char attributename[8];
    strcpy(attributename, "RelName");
    RecId relCatRetValue = linearSearch(RELCAT_RELID,attributename,relNameAttr, EQ);
    if(relCatRetValue.block ==-1 && relCatRetValue.slot==-1)
    {
        return E_RELNOTEXIST;
    }


    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId;
    attrToRenameRecId.block = attrToRenameRecId.slot = -1;
    

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;

        RecId SearchIndex = linearSearch(ATTRCAT_RELID, attributename, relNameAttr, EQ);

        if(SearchIndex.block == -1 && SearchIndex.slot==-1)
        {
            break;
        }
        RecBuffer Blocktosearch(SearchIndex.block);
        
        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        Blocktosearch.getRecord(attrCatRecord, SearchIndex.slot);
        

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0)
        {
            return E_ATTREXIST;
        }
        if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0)
        {
            attrToRenameRecId.block =  SearchIndex.block;
            attrToRenameRecId.slot = SearchIndex.slot;
            break;
        }
       


        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId.block ==-1 && attrToRenameRecId.slot==-1)
    {
        return E_ATTRNOTEXIST;
    }


    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer blockContainingAttr(attrToRenameRecId.block);
    Attribute attrRecord[ATTRCAT_NO_ATTRS];
    blockContainingAttr.getRecord(attrRecord, attrToRenameRecId.slot);
    strcpy(attrRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
    blockContainingAttr.setRecord(attrRecord, attrToRenameRecId.slot);



    return SUCCESS;
}