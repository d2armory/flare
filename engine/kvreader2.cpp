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
	// variable prepare
	char* data = dataH;
	ntroStruct* sEntries = (ntroStruct*) (((char*) &ntroH->structOffset) + ntroH->structOffset);
	ntroStruct* baseStruct = 0;
	
	// Apply base struct structure
	if(str->baseStructId != 0)
	{
		// Find it in NTRO block (move this to its own function?)
		for(int s=1;s<ntroH->structCount;s++)
		{
			ntroStruct* checking = sEntries + s;
			if(checking->id == str->baseStructId)
			{
				baseStruct = checking;
				break;
			}
		}
		// Now apply it
		ApplyStruct(parent, baseStruct, data, rerlH, ntroH);
		// Don't need to apply size to data because diskOffset of field in derived struct
		// already include this padding
	}
	// Apply our structure
	ntroField* fEntry = (ntroField*) (((char*) &str->fieldOffset) + str->fieldOffset);
	for(int k=0;k<str->fieldCount;k++)
	{
		// variable prepare
		ntroField* f = fEntry + k;
		char* dataF = data + f->diskOffset;
		// create node, assign name, hash, type
		KeyValue* node = new KeyValue();
		char* fieldName = ((char*) &f->nameOffset) + f->nameOffset;
		node->key = fieldName;
		node->CalcHash();
		node->type = f->type;
		parent->Attach(node);
		
		// prepare just in case
		char* indirect =  ((char*) &f->indirectOffset) + f->indirectOffset;
		
		// check for special type
		if(f->indirectLevel > 0)// && indirect[0] == 0x04)
		{
			// Array of data
			
			// structure: { 4:offset, 4:count }
			char* elemPointer = dataF + *((unsigned int*) dataF);
			unsigned int elemCount = *((unsigned int*) (dataF + 4));
			if(indirect[0] == 0x03) elemCount = 1;
			
			if(elemCount<=0)
			{
				printf("Got %d size array, skippping\n",elemCount);
				continue;
			}
			
			if(elemCount>100)
			{
				// why it has to put all vertex in structure ...
				elemCount = 100;
			}
			
			node->value = 0;
			
			if(f->type==0x01)
			{
				// finding the structure used for this data from NTRO blocks
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
				// loop over each element
				for(int e=0;e<elemCount;e++)
				{
					// new element node
					KeyValue* elemNode = new KeyValue();
					node->Attach(elemNode);
					// apply it
					ApplyStruct(elemNode, baseStruct, elemPointer, rerlH, ntroH);
					// move pointer to next element
					elemPointer += baseStruct->size;
				}
			}
			else
			{
				//printf("field %s type %d count %d\n",((char*) (&f->nameOffset)) + f->nameOffset,f->type,elemCount);
			
				// Apply current field over array
				int fieldSize = 0;
				if(f->type==NTRO_DATA_TYPE_HANDLE)
				{
					fieldSize = 8;
				}
				else if(f->type==NTRO_DATA_TYPE_NAME)
				{
					fieldSize = 4;
				}
				else if(f->type==NTRO_DATA_TYPE_SHORT || f->type==NTRO_DATA_TYPE_USHORT)
				{
					fieldSize = 2;
				}
				else if(f->type==NTRO_DATA_TYPE_INTEGER || f->type==NTRO_DATA_TYPE_UINTEGER || f->type==NTRO_DATA_TYPE_FLOAT)
				{
					fieldSize = 4;
				}
				if(fieldSize==0)
				{
					fieldSize = 8;
					//printf("Need field type for non-handle %d!!\n",f->type);
				}
				for(int e=0;e<elemCount;e++)
				{
					// new element node
					KeyValue* elemNode = new KeyValue();
					node->Attach(elemNode);
					elemNode->type = f->type;
					elemNode->key = 0;
					// apply it
					ApplyField(elemNode, f, elemPointer, rerlH, ntroH);
					// move pointer to next element
					elemPointer += fieldSize;
				}
			}
		}
		else
		{
			
			if(f->type==0x01)
			{
				// child struct
				for(int s=1;s<ntroH->structCount;s++)
				{
					ntroStruct* checking = sEntries + s;
					if(checking->id == f->typeData)
					{
						baseStruct = checking;
						break;
					}
				}
				// Now apply it
				node->value = 0;
				ApplyStruct(node, baseStruct, dataF, rerlH, ntroH);
			}
			else
			{
				// Direct value
				ApplyField(node, f, dataF,rerlH,ntroH);
				// add node to parent
			}
		}
	}
}

void KVReader2::ApplyField(KeyValue* node, ntroField* f, char* dataF, rerlHeader* rerlH, ntroHeader* ntroH)
{
	// rememeber: node->value is a pointer, not an actual value
	node->value = dataF;
	// special type
	if(node->type == NTRO_DATA_TYPE_NAME)
	{
		// solve string reference
		node->value = node->value + *((unsigned int*) node->value);
	}
	else if(node->type == NTRO_DATA_TYPE_HANDLE)
	{
		// solve external reference
		// prepare hash value
		int* h0 = (int*) node->value;
		int* h1 = h0 + 1;
		// loop over RERL record
		rerlRecord* recEntries = (rerlRecord*) (((char*) &rerlH->recordOffset) + rerlH->recordOffset);
		for(int r=0;r<rerlH->recordCount;r++)
		{
			rerlRecord* rec = recEntries + r;
			if((*h0) == rec->id[0] && (*h1) == rec->id[1])
			{
				// found it
				node->value = ((char*) &rec->nameOffset) + rec->nameOffset;
				break;
			}
		}
	}
}

void KVReader2::Clean(KeyValue* pData)
{
	// Recursively delete node
	KeyValue* cur = pData;
	while(cur!=0)
	{
		Clean(cur->child);
		KeyValue* tmp = cur->sibling;
		delete cur;
		cur = tmp;
	}
}

void KVReader2::Dump(KeyValue* inNode)
{
	KeyValue* cur = inNode;

	while(cur!=0) 
	{
	
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
				printf(" str:'%s'",cur->AsHandle());
			}
			else if(cur->type==NTRO_DATA_TYPE_INTEGER)
			{
				printf(" int:%d",cur->AsInt());
			}
			else if(cur->type==NTRO_DATA_TYPE_UINTEGER)
			{
				printf(" uint:%u",cur->AsUint());
			}
			else if(cur->type==NTRO_DATA_TYPE_SHORT)
			{
				printf(" short:%d",cur->AsShort());
			}
			else if(cur->type==NTRO_DATA_TYPE_USHORT)
			{
				printf(" ushort:%u",cur->AsUshort());
			}
			else if(cur->type==NTRO_DATA_TYPE_BYTE)
			{
				printf(" char:%u",cur->AsByte());
			}
			else if(cur->type==NTRO_DATA_TYPE_FLOAT)
			{
				printf(" float:%f",cur->AsFloat());
			}
			else if(cur->type==10)	// not sure what it is, but it's used as image format
			{
				printf(" format:%u",cur->AsByte());
			}
			else
			{
				printf(" 0x%X [t=%d]",(unsigned int) cur->value, cur->type);
			}
		}
		else
		{
			printf(" [%d]",cur->childCount);
		}
		printf("\n");
		
		Dump(cur->child);
	
		cur = cur->sibling;
	
	}
	
}