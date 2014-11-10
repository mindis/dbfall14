#include <iostream>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "ix.h"
#include "ixtest_util.h"

IndexManager *indexManager;

int testCase_8(const string &indexFileName, const Attribute &attribute)
{
    // Functions tested
    // 1. Create Index File
    // 2. OpenIndex File
    // 3. Insert entry
    // 4. Scan entries, and delete entries
    // 5. Scan close
    // 6. Insert entries again
    // 7. Scan entries
    // 8. CloseIndex File
    // 9. DestroyIndex File
    // NOTE: "**" signifies the new functions being tested in this test case.
    cout << endl << "****In Test Case 8****" << endl;

    RC rc;
    RID rid;
    IXFileHandle ixfileHandle;
    IX_ScanIterator ix_ScanIterator;
    float compVal;
    unsigned numOfTuples;
    unsigned numberOfPages = 4;

    float key;

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
    numOfTuples = 200;
    for(unsigned i = 1; i <= numOfTuples; i++)
    {
        key = (float)i;
        rid.pageNum = i;
        rid.slotNum = i;

        rc = indexManager->insertEntry(ixfileHandle, attribute, &key, rid);
        if(rc != success)
        {
            cout << "Failed Inserting Keys..." << endl;
        	indexManager->closeFile(ixfileHandle);
        	return fail;
        }
    }

    //scan
    compVal = 200.0;
    rc = indexManager->scan(ixfileHandle, attribute, NULL, &compVal, true, true, ix_ScanIterator);
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

    // Test DeleteEntry in IndexScan Iterator
    while(ix_ScanIterator.getNextEntry(rid, &key) == success)
    {
        cout << rid.pageNum << " " << rid.slotNum << endl;

        key = (float)rid.pageNum;
        rc = indexManager->deleteEntry(ixfileHandle, attribute, &key, rid);
        if(rc != success)
        {
            cout << "Failed deleting entry in Scan..." << endl;
        	ix_ScanIterator.close();
        	return fail;
        }
    }
    cout << endl;

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

    // insert entry Again
    numOfTuples = 500;
    for(unsigned i = 1; i <= numOfTuples; i++)
    {
        key = (float)i;
        rid.pageNum = i;
        rid.slotNum = i;

        rc = indexManager->insertEntry(ixfileHandle, attribute, &key, rid);
        if(rc != success)
        {
            cout << "Failed Inserting Keys..." << endl;
        	indexManager->closeFile(ixfileHandle);
        	return fail;
        }
    }

    //scan
    compVal = 450.0;
    rc = indexManager->scan(ixfileHandle, attribute, &compVal, NULL, false, true, ix_ScanIterator);
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

    while(ix_ScanIterator.getNextEntry(rid, &key) == success)
    {
        cout << rid.pageNum << " " << rid.slotNum << endl;

        if(rid.pageNum <= 450 || rid.slotNum <= 450)
        {
            cout << "Wrong entries output...failure" << endl;
        	ix_ScanIterator.close();
        	return fail;
        }
    }
    cout << endl;

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

    //close index file(s)
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

    //destroy index file(s)
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

	const string indexFileName = "height_idx";
	Attribute attrHeight;
	attrHeight.length = 4;
	attrHeight.name = "height";
	attrHeight.type = TypeReal;

	RC result = testCase_8(indexFileName, attrHeight);
    if (result == success) {
    	cout << "IX_Test Case 8 passed" << endl;
    	return success;
    } else {
    	cout << "IX_Test Case 8 failed" << endl;
    	return fail;
    }

}
