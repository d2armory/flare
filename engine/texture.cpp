
#include "texture.hpp"

Texture::Texture(const char* fileName)
{
	strncpy(this->fileName,fileName,TEXTURE_NAME_LENGTH);
	std::string fnStr(fileName);
	std::hash<std::string> hasher;
	this->fnHash = (unsigned int) hasher(fnStr);
	state = FS_UNINIT;
	textureData = 0;
}
Texture::~Texture()
{
	if(this->textureId)
	{
		glDeleteTextures(1, &this->textureId);
	}
	if(textureData != 0) free(textureData);
}

void Texture::Update()
{
	if(state == FS_UNINIT)
	{
		state = FS_LOADING;
		if(!FileLoader::FileExist(fileName))
		{
			FileLoader::Load(fileName);
		}
	}
	if(state == FS_LOADING)
	{
		if(FileLoader::FileExist(fileName))
		{
			state = FS_READY;
			
			char* txtData = textureData = FileLoader::ReadFile(fileName);
			vtfHeader* txtHeader = (vtfHeader*) txtData;
			
			printf("texture: %s\n",fileName);
			printf("- signature: 0x%X\n",(unsigned int) txtHeader->signature);
			printf("- version: %d.%d\n",txtHeader->version[0],txtHeader->version[1]);
			printf("- header size: %d\n",txtHeader->headerSize);
			printf("- size: WxH =  %dx%d\n",txtHeader->width,txtHeader->height);
			printf("- num frame: %d\n",txtHeader->frames);
			printf("- high res format: %d\n",txtHeader->highResImageFormat);
			printf("- mipmap count: %d\n",txtHeader->mipmapCount);
			//printf("- high res format: %d\n",*((int*) (((char*) (&txtHeader->highResImageFormat)) + 5)));
			printf("- low res format: %d\n",txtHeader->lowResImageFormat);
			printf("- depth: %d\n",txtHeader->depth);
			printf("- num resources: %d\n",txtHeader->numResources);
			
			unsigned int reiPad = 80;	// entry start at 80 from file pointer
			
			for(int r=0;r<txtHeader->numResources;r++)
			{
				vtfResouceEntryInfo* rei = (vtfResouceEntryInfo*) (txtData + reiPad + (sizeof(vtfResouceEntryInfo) * r));
				printf("--- res #%d: 0x%X (%c%c%c)\n",r,rei->eType,rei->chTypeBytes[0],rei->chTypeBytes[1],rei->chTypeBytes[2]);
				if((rei->chTypeBytes[3] & 0x02) != 0)
				{
					printf("----- data: 0x%X\n",rei->offset);
				}
				if(rei->eType==0x30)
				{
					char* entry = txtData + rei->offset;
					
					// disable for now
					if(false && txtHeader->highResImageFormat != IMAGE_FORMAT_DXT5)
					{
						printf("----- image not in dxt5 format, abort\n");
					}
					else
					{
						
						glGenTextures(1, &this->textureId);
						glBindTexture(GL_TEXTURE_2D, this->textureId);
						
						// TODO: use texture flag
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						
						int* imgSize = new int[txtHeader->mipmapCount * 4];
						
						int tmpH = txtHeader->height;
						int tmpW = txtHeader->width;
						
						int offset = 0;
						
						unsigned int sqImageType = squish::kDxt1;
						
						switch(txtHeader->highResImageFormat)
						{
							case IMAGE_FORMAT_DXT1:
								sqImageType = squish::kDxt1;
								break;
							case IMAGE_FORMAT_DXT5:
								sqImageType = squish::kDxt5;
								break;
							default:
								sqImageType = 1 << 31;
								printf("----- unspoorted image format %d\n",txtHeader->highResImageFormat);
								break;
						}
						
						for(int i = 0; i < txtHeader->mipmapCount ; i++ )
						{
							// for this level
							//int size = ((tmpW + 3) / 4) * ((tmpH + 3) / 4) * 16;
							int compressedSize =  squish::GetStorageRequirements( tmpW, tmpH, sqImageType );
							imgSize[i*4 + 0] = tmpH;
							imgSize[i*4 + 1] = tmpW;
							imgSize[i*4 + 2] = compressedSize;
							//imgSize[i*4 + 3] = offset;
							// for next level
							//offset += size;
							tmpW = tmpW >> 1;
							tmpH = tmpH >> 1;
							if(tmpW<1) tmpW = 1;
							if(tmpH<1) tmpH = 1;
						}
						// TODO: enable cubemap support (offset *= 6, bind to cubemap texture, etc)
						for(int j = txtHeader->mipmapCount - 1; j >= 0; j--)
						{
							imgSize[j*4 + 3] = offset;
							offset += imgSize[j*4 + 2];
						}
						
						for(int m=0;m<txtHeader->mipmapCount;m++)
						{
							char* data = entry + imgSize[m*4+3];
							
							if(txtHeader->highResImageFormat == IMAGE_FORMAT_DXT1)
							{
								glCompressedTexImage2D(	GL_TEXTURE_2D, m, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
							
								printf("----- mm #%d : %dx%d , dxtSize: %d (webgl dxt1)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2]);
								
							}
							else
							{
								// sadly, webgl doesn't support dxt5
								// we have to unpack this on the fly :(
								//glCompressedTexImage2D(	GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, imgSize[i*3+1], imgSize[i*3+0], 0, imgSize[i*3+2], data);
							
								// rgba
								int allocSize = imgSize[m*4 + 0] * imgSize[m*4 + 1] * 4;
								
								printf("----- mm #%d : %dx%d , dxtSize: %d, rgbaSize: %d (webgl no dxt5 support)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2],allocSize);
								
								unsigned char* unpacked = (unsigned char*) malloc(allocSize);
								
								squish::DecompressImage( unpacked, imgSize[m*4 + 1], imgSize[m*4 + 0], data, sqImageType );
								
								glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, imgSize[m*4 + 1], imgSize[m*4 + 0], 0, GL_RGBA, GL_UNSIGNED_BYTE, unpacked);
								
								/* if(m==0)
								{
									for(int b=0;b<allocSize;b++)
									{
										//printf("%X",unpacked[b]);
										//if(b%32==31) printf("\n");
									}
									//printf("\n");
								} */
								
								free(unpacked);
							}
							
						}
						
						glBindTexture(GL_TEXTURE_2D, 0);
						
					}
					
				}
			}
			
			printf("Texture %s ready\n",fileName);
		}
	}
}

void Texture::Bind(int i)
{
	if(state == FS_READY)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, this->textureId);
	}
}
void Texture::Unbind(int i)
{
	if(state == FS_READY)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}