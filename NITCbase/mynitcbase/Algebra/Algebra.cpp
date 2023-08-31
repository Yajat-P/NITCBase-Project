#include "Algebra.h"

#include <cstring>
#include <iostream>

using namespace std;


bool isNumber(char *str) {
  int len;
  float ignore;
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}


int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
  if (ret != SUCCESS) {
    return ret;
  }

  int type = attrCatEntry.attrType;
  Attribute attrVal;
  if (type == NUMBER) {
    if (isNumber(strVal)) {
      attrVal.nVal = atof(strVal);
    } else {
      return E_ATTRTYPEMISMATCH;
    }
  } else if (type == STRING) {
    strcpy(attrVal.sVal, strVal);
  }

  // Before calling the search function, reset the search to start from the first hit
  int afra = RelCacheTable::resetSearchIndex(srcRelId);
  if (afra != SUCCESS) {
    return afra;
  }

  RelCatEntry relCatEntry;
  ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
  if (ret != SUCCESS) {
    return ret;
  }

  printf("|");
  for (int i = 0; i < relCatEntry.numAttrs; ++i) {
    AttrCatEntry attrCatEntry;
    ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
    if (ret != SUCCESS) {
      return ret;
    }

    printf(" %s |", attrCatEntry.attrName);
  }
  printf("\n");

  while (true) {
    RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

    if (searchRes.block != -1 && searchRes.slot != -1) {
      RecBuffer yajat(searchRes.block);

      struct HeadInfo head;
      yajat.getHeader(&head);
      Attribute record[head.numAttrs];
      yajat.getRecord(record, searchRes.slot);  //CHECK THIS NIGA 

      printf("|");
      for (int i = 0; i < head.numAttrs; ++i) {
        AttrCatEntry attrbu;
        AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrbu);
        if (attrbu.attrType == NUMBER) {
          printf(" %d |", (int)record[i].nVal);
        } else {
          printf(" %s |", record[i].sVal);
        }
      }
      printf("\n");

    } else {
      break;
    }
  }

  return SUCCESS;
}

