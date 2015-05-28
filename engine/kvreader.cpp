
#include "kvreader.hpp"

PCST* KVReader::Parse(char* data, int fSize)
{
	PCST* root = new PCST();
	
	bool isInData = false;
	bool isInQuote = false;
	unsigned int quoteStart = 0;
	bool isValue = false;
	bool currentIsComment = false;
	unsigned int depth = 0;
	PCST* curNode = root;
	
	for(unsigned int i=0; i<fSize; i++)
	{
		char cur = data[i];
		if(isInData)
		{
			unsigned int quoteEnd = 0;
			if(isInQuote)
			{
				if(cur == '"')
				{
					isInData = false;
					isInQuote = false;
					quoteEnd = i;
				}
			}
			else
			{
				if(cur == ' ' || cur == '\r' || cur == '\n' || cur == '\t')
				{
					isInData = false;
					isInQuote = false;
					quoteEnd = i;
				}
			}
			if(quoteEnd != 0)
			{
				if(!isValue)
				{
					PCST* node = new PCST();
					isValue = true;
					node->keylength = quoteEnd - quoteStart;
					node->key = (char*) new char[node->keylength+1];
					strncpy(node->key, data+quoteStart, node->keylength);
					node->key[node->keylength] = '\0';
					node->depth = depth;
					
					for(unsigned int a = 0; a < node->keylength; a++)
					{
						node->key[a] = tolower(node->key[a]);
					}
					
					node->parent = curNode->parent;
					/* node->sibling = curNode;
					if(curNode->parent)
					{
						curNode->parent->child = node;
					}
					else
					{
						root = node;
					}
					*/
					curNode->sibling = node;
					curNode = node;
					//printf("New node: key = %s\n",node->key);
				}
				else
				{
					isValue = false;
					curNode->valuelength = quoteEnd - quoteStart;
					curNode->value = (char*) new char[curNode->valuelength+1];
					strncpy(curNode->value, data+quoteStart, curNode->valuelength);
					curNode->value[curNode->valuelength] = '\0';
					//printf("value = %s\n",curNode->value);
				}
			}
		}
		else
		{
			if(cur == '"' && !currentIsComment)
			{
				isInData = true;
				isInQuote = true;
				quoteStart = i + 1;
			}
			else if(cur == ' ' || cur == '\t')
			{
				// ignore
			}
			else if(cur == '\r' || cur == '\n')
			{
				// normally ignored except comment end
				currentIsComment = false;
			}
			else if(cur == '{' && !currentIsComment)
			{
				// assume isValue = true
				isValue = false;
				depth++;
				PCST* node = new PCST();
				node->parent = curNode;
				node->depth = depth;
				curNode->child = node;
				curNode = node;
				//printf("Child block start\n");
			}
			else if(cur == '}' && !currentIsComment)
			{
				// assume isValue = false
				depth--;
				curNode = curNode->parent;
				//if(cur==0) cur = root;
				//printf("Child block ended\n");
			}
			else if(cur == '/')
			{
				if(i>0 && data[i-1] == '/')
				{
					currentIsComment = true;
				}
			}
			else if(!currentIsComment)
			{
				isInData = true;
				isInQuote = false;
				quoteStart = i;
			}
		}
	}
	
	//printf("File parse\n");
	
	return root;
}

void KVReader::Clean(PCST* pData)
{
	if(pData==0) return;
	Clean(pData->sibling);
	Clean(pData->child);
	delete pData;
}