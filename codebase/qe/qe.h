#ifndef _qe_h_
#define _qe_h_

#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "../rbf/rbfm.h"
#include "../rm/rm.h"
#include "../ix/ix.h"

# define QE_EOF (-1)  // end of the index scan

using namespace std;

typedef enum{ MIN = 0, MAX, SUM, AVG, COUNT } AggregateOp;


// The following functions use the following
// format for the passed data.
//    For INT and REAL: use 4 bytes
//    For VARCHAR: use 4 bytes for the length followed by
//                 the characters

struct Value {
    AttrType type;          // type of value
    void     *data;         // value
};


struct Condition {
    string  lhsAttr;        // left-hand side attribute
    CompOp  op;             // comparison operator
    bool    bRhsIsAttr;     // TRUE if right-hand side is an attribute and not a value; FALSE, otherwise.
    string  rhsAttr;        // right-hand side attribute if bRhsIsAttr = TRUE
    Value   rhsValue;       // right-hand side value if bRhsIsAttr = FALSE
};


class Iterator {
    // All the relational operators and access methods are iterators.
    public:
        virtual RC getNextTuple(void *data) = 0;
        virtual void getAttributes(vector<Attribute> &attrs) const = 0;
        virtual ~Iterator() {};
};


class TableScan : public Iterator
{
    // A wrapper inheriting Iterator over RM_ScanIterator
    public:
        RelationManager &rm;
        RM_ScanIterator *iter;
        string tableName;
        vector<Attribute> attrs;
        vector<string> attrNames;
        RID rid;

        TableScan(RelationManager &rm, const string &tableName, const char *alias = NULL):rm(rm)
        {
        	//PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::start::TableScan" << endl;
        	//Set members
        	this->tableName = tableName;

            // Get Attributes from RM
            rm.getAttributes(tableName, attrs);

            // Get Attribute Names from RM
            unsigned i;
            for(i = 0; i < attrs.size(); ++i)
            {
                // convert to char *
                attrNames.push_back(attrs[i].name);
            }

            // Call rm scan to get iterator
            iter = new RM_ScanIterator();
            rm.scan(tableName, "", NO_OP, NULL, attrNames, *iter);

            // Set alias
            if(alias) this->tableName = alias;
            //PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::end::TableScan" << endl;
        };

        // Start a new iterator given the new compOp and value
        void setIterator()
        {
        	//PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::start::setIterator" << endl;
            iter->close();
            delete iter;
            iter = new RM_ScanIterator();
            rm.scan(tableName, "", NO_OP, NULL, attrNames, *iter);
            //PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::end::setIteraror" << endl;
        };

        RC getNextTuple(void *data)
        {
        	//PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::start::getNextTuple" << endl;
            return iter->getNextTuple(rid, data);
            //PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::end::getNextTuple" << endl;
        };

        void getAttributes(vector<Attribute> &attrs) const
        {
        	//PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::start::getAttributes" << endl;
            attrs.clear();
            attrs = this->attrs;
            unsigned i;

            // For attribute in vector<Attribute>, name it as rel.attr
            for(i = 0; i < attrs.size(); ++i)
            {
                string tmp = tableName;
                tmp += ".";
                tmp += attrs[i].name;
                attrs[i].name = tmp;
            }
            //PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::end::getAttributes" << endl;
        };

        ~TableScan()
        {
        	//PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::start::dtor" << endl;
        	iter->close();
        	//PagedFileManager::instance()->getNumOpenInstances("employee"); cout << "::end::dtor" << endl;
        };
};


class IndexScan : public Iterator
{
    // A wrapper inheriting Iterator over IX_IndexScan
    public:
        RelationManager &rm;
        RM_IndexScanIterator *iter;
        string tableName;
        string attrName;
        vector<Attribute> attrs;
        char key[PAGE_SIZE];
        RID rid;

        IndexScan(RelationManager &rm, const string &tableName, const string &attrName, const char *alias = NULL):rm(rm)
        {
        	// Set members
        	this->tableName = tableName;
        	this->attrName = attrName;


            // Get Attributes from RM
            rm.getAttributes(tableName, attrs);

            // Call rm indexScan to get iterator
            iter = new RM_IndexScanIterator();
            rm.indexScan(tableName, attrName, NULL, NULL, true, true, *iter);

            // Set alias
            if(alias) this->tableName = alias;
        };

        // Start a new iterator given the new key range
        void setIterator(void* lowKey,
                         void* highKey,
                         bool lowKeyInclusive,
                         bool highKeyInclusive)
        {
            iter->close();
            delete iter;
            iter = new RM_IndexScanIterator();
            rm.indexScan(tableName, attrName, lowKey, highKey, lowKeyInclusive,
                           highKeyInclusive, *iter);
        };

        RC getNextTuple(void *data)
        {
            int rc = iter->getNextEntry(rid, key);
            if(rc == 0)
            {
                rc = rm.readTuple(tableName.c_str(), rid, data);
            }
            return rc;
        };

        void getAttributes(vector<Attribute> &attrs) const
        {
            attrs.clear();
            attrs = this->attrs;
            unsigned i;

            // For attribute in vector<Attribute>, name it as rel.attr
            for(i = 0; i < attrs.size(); ++i)
            {
                string tmp = tableName;
                tmp += ".";
                tmp += attrs[i].name;
                attrs[i].name = tmp;
            }
        };

        ~IndexScan()
        {
            iter->close();
        };
};


class Filter : public Iterator {
    // Filter operator
    public:
        Filter(Iterator *input,               // Iterator of input R
               const Condition &condition     // Selection condition
        );
        ~Filter();

        RC getNextTuple(void *data);
        // For attribute in vector<Attribute>, name it as rel.attr
        void getAttributes(vector<Attribute> &attrs) const;

    private:
        Iterator *iterator; //iterator (Table or Index)
        Value rightHand; // right hand (data and type)
        vector<Attribute> attrs; //vector of attributes
        unsigned position; // position to compare
        CompOp compOp; //comparator
        void * leftHandValue; //left hand value to compare
        void * rightHandValue; //right hand value to compare
};

bool compareTwoField(const void *valueToCompare, const void *condition, AttrType type,
		CompOp compOp);


class Project : public Iterator {
    // Projection operator
    public:
        Project(Iterator *input,                    // Iterator of input R
              const vector<string> &attrNames);   // vector containing attribute names
        ~Project();

        RC getNextTuple(void *data);
        // For attribute in vector<Attribute>, name it as rel.attr
        void getAttributes(vector<Attribute> &attrs) const;

    private:
           Iterator *iterator; //iterator (Table or Index)
           vector<Attribute> attrs; // vector of attributes to project
           vector<Attribute> originalAttrs; // vector of original attributes


};

class GHJoin : public Iterator {
    // Grace hash join operator
    public:
      GHJoin(Iterator *leftIn,               // Iterator of input R
            Iterator *rightIn,               // Iterator of input S
            const Condition &condition,      // Join condition (CompOp is always EQ)
            const unsigned numPartitions     // # of partitions for each relation (decided by the optimizer)
      );
      ~GHJoin();

      RC getNextTuple(void *data);
      // For attribute in vector<Attribute>, name it as rel.attr
      void getAttributes(vector<Attribute> &attrs) const;
};


class BNLJoin : public Iterator {
    // Block nested-loop join operator
    public:
        BNLJoin(Iterator *leftIn,            // Iterator of input R
               TableScan *rightIn,           // TableScan Iterator of input S
               const Condition &condition,   // Join condition
               const unsigned numRecords     // # of records can be loaded into memory, i.e., memory block size (decided by the optimizer)
        );
        ~BNLJoin();

        RC getNextTuple(void *data);
        // For attribute in vector<Attribute>, name it as rel.attr
        void getAttributes(vector<Attribute> &attrs) const;
};


class INLJoin : public Iterator {
    // Index nested-loop join operator
    public:
        INLJoin(Iterator *leftIn,           // Iterator of input R
               IndexScan *rightIn,          // IndexScan Iterator of input S
               const Condition &condition   // Join condition
        );
        ~INLJoin();

        RC getNextTuple(void *data);
        // For attribute in vector<Attribute>, name it as rel.attr
        void getAttributes(vector<Attribute> &attrs) const;
};


class Aggregate : public Iterator {
    // Aggregation operator
    public:
        // Mandatory for graduate teams only
        // Basic aggregation
        Aggregate(Iterator *input,          // Iterator of input R
                  Attribute aggAttr,        // The attribute over which we are computing an aggregate
                  AggregateOp op            // Aggregate operation
        );

        // Optional for everyone. 5 extra-credit points
        // Group-based hash aggregation
        Aggregate(Iterator *input,             // Iterator of input R
                  Attribute aggAttr,           // The attribute over which we are computing an aggregate
                  Attribute groupAttr,         // The attribute over which we are grouping the tuples
                  AggregateOp op,              // Aggregate operation
                  const unsigned numPartitions // Number of partitions for input (decided by the optimizer)
        );
        ~Aggregate();

        RC getNextTuple(void *data);
        // Please name the output attribute as aggregateOp(aggAttr)
        // E.g. Relation=rel, attribute=attr, aggregateOp=MAX
        // output attrname = "MAX(rel.attr)"
        void getAttributes(vector<Attribute> &attrs) const;
    private:
        void determineOutputType(AttrType& type, int& size);
        RC getOffsetToProperField(const void* record, const vector<Attribute> recordDesc, const Attribute properField, int& offset);
        string generateOpName();
        string generatePartitionName(unsigned int partitionNumber);
        RC next_groupBy(void* data);
        RC next_basic(void* data);
        RC nextBucket();
        //data member used solely by basic aggregation
        bool _isFinishedProcessing;	//for basic aggregate this becomes true after the first iteration
        //data members used solely for group-by aggregation
        Attribute _groupByAttr;
        unsigned int _numOfPartitions;
        vector<Attribute> _groupByDesc;
        vector<string> _groupByNames;
        RBFM_ScanIterator _groupByIterator;
        FileHandle _groupByHandle;
        int _groupByCurrentBucketNumber;
        //shared data members between group-by and basic aggregations
        vector<Attribute> _attributes;
        Iterator* _inputStream;
        bool _isGroupByAggregation;
        Attribute _aggAttr;
        AggregateOp _aggOp;
        vector<Attribute> _inputStreamDesc;
};

#endif
