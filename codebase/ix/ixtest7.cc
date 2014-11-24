#include <iostream>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "ix.h"
#include "ixtest_util.h"

IndexManager *indexManager;

int testCase_7(const string &indexFileName, const Attribute &attribute)
{
	// Checks whether duplicated entries are handled properly or not.
	//
	// Functions tested
    // 1. Create Index
    // 2. OpenIndex
    // 3. Insert entry
    // 4. Scan entries (EXACT MATCH).
    // 5. Scan close
    // 6. CloseIndex
    // 7. DestroyIndex
    // NOTE: "**" signifies the new functions being tested in this test case.
    cout << endl << "****In Test Case 7****" << endl;

    RC rc;
    RID rid;
    unsigned numOfTuples;
    unsigned key;
    unsigned numberOfPages = 4;
    IXFileHandle ixfileHandle;
    IX_ScanIterator ix_ScanIterator;
    int compVal;
    int count = 0;

    //create index file(s)
    rc = indexManager->createFile(indexFileName, numberOfPages);
    if(rc == success)
    {
        cout << "Index Created!" << endl;
    }
    else
    {
        cout << "Failed Creating Index File..." << endl;
    	return fail;
    }

    //open index file
    rc = indexManager->openFile(indexFileName, ixfileHandle);
    if(rc == success)
    {
        cout << "Index File Opened!" << endl;
    }
    else
    {
        cout << "Failed Opening Index File..." << endl;
    	indexManager->destroyFile(indexFileName);
    	return fail;
    }

    // insert entry
    numOfTuples = 2000;
    for(unsigned i = 1; i <= numOfTuples; i++)
    {
        key = 1234;
        rid.pageNum = i;
        rid.slotNum = i;

        /*if( i == 1633 || i == 613 || i == 612 || i == 614 || i == 615 || i == 816 || i == 817 || i == 818 || i == 819 )
        {
        	cout << "i = " << i << endl;
            unsigned int numberOfPagesFromFunction = 0;
        	// Get number of primary pages
            rc = indexManager->getNumberOfPrimaryPages(ixfileHandle, numberOfPagesFromFunction);
            if(rc != success)
            {
            	cout << "getNumberOfPrimaryPages() failed." << endl;
            	indexManager->closeFile(ixfileHandle);
        		return fail;
            }

        	// Print Entries in each page
        	for (unsigned i = 0; i < numberOfPagesFromFunction; i++) {
        		rc = indexManager->printIndexEntriesInAPage(ixfileHandle, attribute, i);
        		if (rc != success) {
                	cout << "printIndexEntriesInAPage() failed." << endl;
        			indexManager->closeFile(ixfileHandle);
        			return fail;
        		}
        	}
        }

        if( i == 613 )
        {
        	cout << ">>>>>>> i = " << i << endl;
        }*/

        rc = indexManager->insertEntry(ixfileHandle, attribute, &key, rid);
        if(rc != success)
        {
            cout << "Failed Inserting Keys..." << endl;
        	indexManager->closeFile(ixfileHandle);
        	return fail;
        }
    }

    numOfTuples = 2000;
    for(unsigned i = 500; i < numOfTuples+500; i++)
    {
        key = 4321;
        rid.pageNum = i;
        rid.slotNum = i-5;

        rc = indexManager->insertEntry(ixfileHandle, attribute, &key, rid);
        if(rc != success)
        {
            cout << "Failed Inserting Keys..." << endl;
        	indexManager->closeFile(ixfileHandle);
        	return fail;
        }
    }

    //std::cout << endl << "primary data file:" << endl;
	//printFile(ixfileHandle._primBucketDataFileHandler);
	//std::cout << endl << "overflow data file:" << endl;
	//printFile(ixfileHandle._overBucketDataFileHandler);
    /*unsigned int numberOfPagesFromFunction = 0;
	// Get number of primary pages
    rc = indexManager->getNumberOfPrimaryPages(ixfileHandle, numberOfPagesFromFunction);
    if(rc != success)
    {
    	cout << "getNumberOfPrimaryPages() failed." << endl;
    	indexManager->closeFile(ixfileHandle);
		return fail;
    }

	// Print Entries in each page
	for (unsigned i = 0; i < numberOfPagesFromFunction; i++) {
		rc = indexManager->printIndexEntriesInAPage(ixfileHandle, attribute, i);
		if (rc != success) {
        	cout << "printIndexEntriesInAPage() failed." << endl;
			indexManager->closeFile(ixfileHandle);
			return fail;
		}
	}*/

    //scan
    compVal = 1234;
    rc = indexManager->scan(ixfileHandle, attribute, &compVal, &compVal, true, true, ix_ScanIterator);
    if(rc == success)
    {
        cout << "Scan Opened Successfully!" << endl;
    }
    else
    {
        cout << "Failed Opening Scan..." << endl;
    	indexManager->closeFile(ixfileHandle);
    	return fail;
    }

    //iterate
    count = 0;
    while(ix_ScanIterator.getNextEntry(rid, &key) == success)
    {
        if(count % 1000 == 0)
            cout << "returned rid: " << rid.pageNum << " " << rid.slotNum << endl;
        count++;
    }
    cout << "Number of scanned entries: " << count << endl;
    if (count != 2000)
    {
        cout << "Wrong entries output...failure" << endl;
    	ix_ScanIterator.close();
    	return fail;
    }

    //close scan
    rc = ix_ScanIterator.close();
    if(rc == success)
    {
        cout << "Scan Closed Successfully!" << endl;
    }
    else
    {
        cout << "Failed Closing Scan..." << endl;
    	indexManager->closeFile(ixfileHandle);
    	return fail;
    }

    //close index file file
    rc = indexManager->closeFile(ixfileHandle);
    if(rc == success)
    {
        cout << "Index File Closed Successfully!" << endl;
    }
    else
    {
        cout << "Failed Closing Index File..." << endl;
    	indexManager->destroyFile(indexFileName);
    	return fail;
    }

    //destroy index file file
    rc = indexManager->destroyFile(indexFileName);
    if(rc == success)
    {
        cout << "Index File Destroyed Successfully!" << endl;
    }
    else
    {
        cout << "Failed Destroying Index File..." << endl;
    	return fail;
    }

    return success;

}

int main()
{
    //Global Initializations
    indexManager = IndexManager::instance();

	const string indexFileName = "age_idx";
	Attribute attrAge;
	attrAge.length = 4;
	attrAge.name = "age";
	attrAge.type = TypeInt;

	RC result = testCase_7(indexFileName, attrAge);
    if (result == success) {
    	cout << "IX_Test Case 7 passed." << endl;
    	return success;
    } else {
    	cout << "IX_Test Case 7 failed. Duplicated entries are not handled properly." << endl;
    	return fail;
    }

}

