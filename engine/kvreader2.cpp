#include "kvreader2.hpp"

KeyValue::KeyValue()
{
	parent = 0;
	child = 0;
	sibling = 0;
	keyHash = 0;
	
	key = (char*) 0;
	value = (char*) 0;
	
	depth = 0;
	type = 0;
	childCount = 0;
	
}

KeyValue::~KeyValue()
{
	// don't need to delete key/value anymore because it's not copied
}

void KeyValue::CalcHash()
{
	
	if(key!=0)
	{
		std::string fnStr(key);
		std::hash<std::string> hasher;
		keyHash = (unsigned int) hasher(fnStr);
	}
	else
	{
		keyHash = 0;
	}
}

void KeyValue::Attach(KeyValue* child)
{
	this->childCount++;
	child->parent = this;
	child->depth = this->depth + 1;
	if(this->child == 0)
	{
		this->child = child;
	}
	else
	{
		// add at end of list for easy human reading
		KeyValue* cur = this->child;
		while(cur->sibling !=0)
		{
			cur = cur->sibling;
		}
		cur->sibling = child;
	}
}

KeyValue* KeyValue::Get(int index) const
{
	KeyValue* cur = this->child;
	for(int i=0;i<index;i++)
	{
		if(cur) cur = cur->sibling;
	}
	return cur;
}

KeyValue* KeyValue::Find(const char* name) const
{
	std::string fnStr(name);
	std::hash<std::string> hasher;
	unsigned int hash = (unsigned int) hasher(fnStr);
	
	KeyValue* cur = this->child;
	while(cur !=0)
	{
		if(hash == cur->keyHash)
		{
			if(fnStr.compare(std::string(cur->key))==0)
			{
				return cur;
			}
		}
		cur = cur->sibling;
	}
	return 0;
}

const char* KeyValue::AsName() const
{
	return value;
}

const char* KeyValue::AsHandle() const
{
	return value;
}

uint8_t KeyValue::AsByte() const
{
	return *((uint8_t*) value);
}

int16_t KeyValue::AsShort() const
{
	return *((int16_t*) value);
}

uint16_t KeyValue::AsUshort() const
{
	return *((uint16_t*) value);
}

int32_t KeyValue::AsInt() const
{
	return *((int32_t*) value);
}

uint32_t KeyValue::AsUint() const
{
	return *((uint32_t*) value);
}

int64_t KeyValue::AsLong() const
{
	return *((int64_t*) value);
}

uint64_t KeyValue::AsUlong() const
{
	return *((uint64_t*) value);
}

float KeyValue::AsFloat() const
{
	return *((float*) value);
}

KeyValue* KVReader2::Parse(char* data)
{
	dmxHeader* dmxH = (dmxHeader*) data;
	// Signature check
	if(dmxH->sig != 0x0c) return 0;
	
	// Reference prepare
	rerlHeader* rerlH = 0;
	ntroHeader* ntroH = 0;
	char* dataH = 0;
	
	// Loop through each block header and assign a correct reference
	for(int blockIndex=0;blockIndex < dmxH->blockCount;blockIndex++)
	{
		dmxBlockHeader* blockH = dmxH->block(blockIndex);
		if(blockH->name[0] == 'D')
		{
			// DATA	
			dataH = (char*) blockH->data();
		}
		else if(blockH->name[0] == 'N')
		{
			// NTRO
			ntroH = blockH->ntroData();
		}
		else if(blockH->name[0] == 'R' && blockH->name[2] == 'R')
		{
			// RERL
			rerlH = blockH->rerlData();
		}
	}
	
	//printf("rerlH = %X, ntroH = %X, dataH = %X\n",(unsigned int) rerlH,(unsigned int) ntroH,(unsigned int) dataH);
	// Assuming file format is correct (NOT a good idea, should add assert)
	
	// Create root node
	KeyValue* root = new KeyValue();
	
	// Apply struct 0 to data
	ntroStruct* str = (ntroStruct*) (((char*) &ntroH->structOffset) + ntroH->structOffset);
	
	ApplyStruct(root, str, dataH, rerlH, ntroH);
	
	return root;
	
}

void KVReader2::ApplyStruct(KeyValue* parent, ntroStruct* str, char* dataH, rerlHeader* rerlH, ntroHeader* ntroH)
{
	
	char* data = dataH;
	ntroStruct* sEntries = (ntroStruct*) (((char*) &ntroH->structOffset) + ntroH->structOffset);
	ntroStruct* baseStruct = 0;
	
	// Apply base struct structure
	if(str->baseStructId != 0)
	{
		for(int s=1;s<ntroH->structCount;s++)
		{
			ntroStruct* checking = sEntries + s;
			if(checking->id == str->baseStructId)
			{
				baseStruct = checking;
				break;
			}
		}
		//printf("Applying base struct %s\n",((char*) &baseStruct->nameOffset) + baseStruct->nameOffset); 
		ApplyStruct(parent, baseStruct, dataH, rerlH, ntroH);
	}
	//printf("Applying current struct %s\n",((char*) &str->nameOffset) + str->nameOffset);
	// Apply our structure
	unsigned int read = 0;	// past-tense, how much data were read
	ntroField* fEntry = (ntroField*) (((char*) &str->fieldOffset) + str->fieldOffset);
	for(int k=0;k<str->fieldCount;k++)
	{
		ntroField* f = fEntry + k;
		
		char* dataF = data + f->diskOffset;
		
		KeyValue* node = new KeyValue();
		char* fieldName = ((char*) &f->nameOffset) + f->nameOffset;
		node->key = fieldName;
		node->CalcHash();
		node->type = f->type;
		
		char* indirect =  ((char*) &f->indirectOffset) + f->indirectOffset;
		if(f->indirectLevel > 0 && indirect[0] == 0x04)
		{
			//printf("Applying array %s\n",((char*) &f->nameOffset) + f->nameOffset);
			// array of data
			// assume it's struct type
			char* elemPointer = dataF + *((unsigned int*) dataF);
			unsigned int elemCount = *((unsigned int*) (dataF + 4));
			baseStruct = 0;
			for(int s=1;s<ntroH->structCount;s++)
			{
				ntroStruct* checking = sEntries + s;
				if(checking->id == f->typeData)
				{
					baseStruct = checking;
					break;
				}
			}
			node->value = 0;
			parent->Attach(node);
			for(int e=0;e<elemCount;e++)
			{
				KeyValue* elemNode = new KeyValue();
				node->Attach(elemNode);
				ApplyStruct(elemNode, baseStruct, elemPointer, rerlH, ntroH);
				elemPointer += baseStruct->size;
			}
		}
		else if(f->indirectLevel > 0 && indirect[1] == 0x03)
		{
			// pointer?
			// dunon how to process this
			printf("Dunno how to process pointer..\n");
		}
		else
		{
			//printf("Applying value %s of type 0x%X\n",((char*) &f->nameOffset) + f->nameOffset,f->type);
			// direct value
			node->value = dataF;
			if(node->type == NTRO_DATA_TYPE_NAME)
			{
				// solve string reference
				node->value = node->value + *((unsigned int*) node->value);
			}
			else if(node->type == NTRO_DATA_TYPE_HANDLE)
			{
				// solve external reference
				int* h0 = (int*) node->value;
				int* h1 = h0 + 1;
				//printf("Processing record 0x%X%X (reverse for little end)\n",*h1,*h0);
				rerlRecord* recEntries = (rerlRecord*) (((char*) &rerlH->recordOffset) + rerlH->recordOffset);
				for(int r=0;r<rerlH->recordCount;r++)
				{
					rerlRecord* rec = recEntries + r;
					//printf("- comparing rec %d : 0x%X%X (reverse for little end) - %s\n",r,rec->id[0],rec->id[1],((char*) &rec->nameOffset) + rec->nameOffset);
					if((*h0) == rec->id[0] && (*h1) == rec->id[1])
					{
						//printf("Found at %d\n",r);
						node->value = ((char*) &rec->nameOffset) + rec->nameOffset;
						break;
					}
				}
			}
			// add node to parent
			parent->Attach(node);
		}
	}
}

void KVReader2::Clean(KeyValue* pData)
{
	if(pData==0) return;
	Clean(pData->sibling);
	Clean(pData->child);
	delete pData;
}

void KVReader2::Dump(KeyValue* cur)
{
	if(cur==0) return;
	for(int i=0;i<cur->depth;i++)
	{
		printf("==");
	}
	if(cur->key!=0)
	{
		printf("= '%s' [%X] :",cur->key,cur->keyHash);
	}
	else 
	{
		printf("= []");
	}
	if(cur->value)
	{
		if(cur->type==NTRO_DATA_TYPE_HANDLE || cur->type==NTRO_DATA_TYPE_NAME)
		{
			printf(" '%s'",cur->AsHandle());
		}
		else
		{
			printf(" 0x%X",(unsigned int) cur->value);
		}
	}
	else
	{
		printf(" [%d]",cur->childCount);
	}
	printf("\n");
	
	Dump(cur->child);
	Dump(cur->sibling);
}