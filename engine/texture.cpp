
#include "texture.hpp"

Texture::Texture(const char* fileName)
{
	strncpy(this->fileName,fileName,TEXTURE_NAME_LENGTH);
	std::string fnStr(fileName);
	std::hash<std::string> hasher;
	this->fnHash = (unsigned int) hasher(fnStr);
	state = FS_UNINIT;
	textureData = 0;
	childLeft = 0;
	childRight = 0;
	nextTexture = 0;
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
		FileLoader::Load(fileName);
	}
	if(state == FS_LOADING)
	{
		if(FileLoader::FileExist(fileName))
		{
			state = FS_READY;
			
			char* txtData = textureData = FileLoader::ReadFile(fileName);
			
			// Check file type
			std::string fName = std::string(fileName);
			if(fName.substr(fName.find_last_of(".") + 1).compare("vtex_c") == 0)
			{
				printf("texture [S2]: %s\n",fileName);
				
				// SOURCE 2 format
				
				DmxHeader* dmxHeader = (DmxHeader*) txtData;
				VtexHeader* vtexHeader = 0;
				unsigned int vtexHeaderSize = 0;
				// Finding data block
				for(int i=0;i<dmxHeader->blockCount;i++)
				{
					DmxBlockHeader* blockHeader = dmxHeader->block(i);
					if(blockHeader->name[0] == 'D')
					{
						vtexHeader = blockHeader->vtexData();
						vtexHeaderSize = blockHeader->dataSize;
					}
				}
				if(vtexHeader == 0)
				{
					printf("Error locating data block!\n");
					return;
				}
				
				printf("- size: WxH =  %dx%d\n",vtexHeader->width,vtexHeader->height);
				//printf("- mipmap count: %d\n",vtexHeader->mipLevel);
				//printf("- format: %d\n",vtexHeader->format);
				
				// variable preparation
				isCubemap = false; // TODO: accept cubemap
				txtType = GL_TEXTURE_2D;
				if(isCubemap) txtType = GL_TEXTURE_CUBE_MAP;
				int numFaces = (isCubemap)?6:1;
				unsigned int sqImageType = squish::kDxt1;	// format in squish type
				switch(vtexHeader->format)
				{
					case IMAGE_FORMAT_DXT1:
						sqImageType = squish::kDxt1;
						break;
					case IMAGE_FORMAT_DXT5:
						sqImageType = squish::kDxt5;
						break;
					default:
						sqImageType = 1 << 31;
						printf("----- unsupported image format %d\n",vtexHeader->format);
						break;
				}
				
				// GL texture generate
				glGenTextures(1, &this->textureId);
				glBindTexture(txtType, this->textureId);
				
				// TODO: use texture flag
				glTexParameteri(txtType, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(txtType, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(txtType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(txtType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				
				// Offset calculation for each mipmap level
				int numMip = vtexHeader->mipLevel;
				// TODO: use struct for this (4x array so ugly)
				int* imgSize = new int[numMip * 4];
				
				// Calculate width, height, size in byte first
				int tmpH = vtexHeader->height;
				int tmpW = vtexHeader->width;
				//printf("%dx%d @ %d\n",tmpW,tmpH,sqImageType);
				for(int i = 0; i < numMip ; i++ )
				{
					// I don't know why I have to x2 here ...
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
				// then start adding up from smallest mipmap (source store smallest one first)
				int offset = 0;
				for(int j = numMip - 1; j >= 0; j--)
				{
					imgSize[j*4 + 3] = offset;
					offset += imgSize[j*4 + 2];
				}
				
				// load data into gpu here
				char* entry = ((char*) txtData) + dmxHeader->fileSize;
				for(int m=0;m<numMip;m++)
				{
					for(int f=0;f<numFaces;f++)
					{
						// get data pointer
						char* data = entry + imgSize[m*4+3];
						//printf("----- mm #%d : %dx%d , dxtSize: %d, startAt: %d (%d from SoF, %d from entry)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2],(unsigned int) data,(unsigned int) (data-txtData),imgSize[m*4+3]);
						
						unsigned int txtTypeFS = txtType;
						
						// Seem to can't get Vtex DXT1 to work with webgl
						// don't know why so disable for time being
						if(Scene::enableTextureCompression && false)
						{
							// Use compressed texture if possible
							const GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;
							if(vtexHeader->format == IMAGE_FORMAT_DXT1)
							{
								glCompressedTexImage2D(	txtTypeFS, m, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
							}
							else if(vtexHeader->format == IMAGE_FORMAT_DXT5)
							{
								glCompressedTexImage2D(	txtTypeFS, m, COMPRESSED_RGBA_S3TC_DXT5_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
							}
						}
						else
						{
							// Compressed texture not supported
							// Unpack them on the fly
							
							// rgba
							int allocSize = imgSize[m*4 + 0] * imgSize[m*4 + 1] * 4;
							unsigned char* unpacked = (unsigned char*) malloc(allocSize);
							
							squish::DecompressImage( unpacked, imgSize[m*4 + 1], imgSize[m*4 + 0], data, sqImageType );
							
							glTexImage2D(txtTypeFS, m, GL_RGBA, imgSize[m*4 + 1], imgSize[m*4 + 0], 0, GL_RGBA, GL_UNSIGNED_BYTE, unpacked);
							
							free(unpacked);
						}
					}
					
				}
				
				// for some reason, vtex seem not to provide all mipmap level
				glGenerateMipmap(GL_TEXTURE_2D);
				
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else
			{
			
				// SOURCE 1
			
				vtfHeader* txtHeader = (vtfHeader*) txtData;
				
				printf("texture: %s\n",fileName);
				//printf("- signature: 0x%X\n",(unsigned int) txtHeader->signature);
				//printf("- version: %d.%d\n",txtHeader->version[0],txtHeader->version[1]);
				//printf("- header size: %d\n",txtHeader->headerSize);
				printf("- size: WxH =  %dx%d\n",txtHeader->width,txtHeader->height);
				//printf("- num frame: %d\n",txtHeader->frames);
				//printf("- high res format: %d\n",txtHeader->highResImageFormat);
				//printf("- mipmap count: %d\n",txtHeader->mipmapCount);
				//printf("- low res format: %d\n",txtHeader->lowResImageFormat);
				//printf("- depth: %d\n",txtHeader->depth);
				//printf("- num resources: %d\n",txtHeader->numResources);
				
				unsigned int reiPad = 80;	// entry start at 80 from file pointer
				
				isCubemap = ((txtHeader->flags & 0x4000) != 0);
				
				txtType = GL_TEXTURE_2D;
				if(isCubemap) txtType = GL_TEXTURE_CUBE_MAP;
				
				int numFaces = (isCubemap)?6:1;
				
				for(int r=0;r<txtHeader->numResources;r++)
				{
					vtfResouceEntryInfo* rei = (vtfResouceEntryInfo*) (txtData + reiPad + (sizeof(vtfResouceEntryInfo) * r));
					//printf("--- res #%d: 0x%X (%c%c%c)\n",r,rei->eType,rei->chTypeBytes[0],rei->chTypeBytes[1],rei->chTypeBytes[2]);
					if((rei->chTypeBytes[3] & 0x02) != 0)
					{
					//	printf("----- data: 0x%X\n",rei->offset);
					}
					if(rei->eType==0x30)
					{
						char* entry = txtData + rei->offset;
						
						// disable for now
						if(!(txtHeader->highResImageFormat == IMAGE_FORMAT_DXT1 || txtHeader->highResImageFormat == IMAGE_FORMAT_DXT5))
						{
							printf("----- Texture not in dxt1/dxt5 format, abort!\n");
						}
						else
						{
							
							glGenTextures(1, &this->textureId);
							glBindTexture(txtType, this->textureId);
							
							// TODO: use texture flag
							glTexParameteri(txtType, GL_TEXTURE_WRAP_S, GL_REPEAT);
							glTexParameteri(txtType, GL_TEXTURE_WRAP_T, GL_REPEAT);
							glTexParameteri(txtType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							glTexParameteri(txtType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
							
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
									printf("----- unsupported image format %d\n",txtHeader->highResImageFormat);
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
								offset += imgSize[j*4 + 2] * numFaces;
							}
							
							for(int m=0;m<txtHeader->mipmapCount;m++)
							{
								for(int f=0;f<numFaces;f++)
								{
									char* data = entry + imgSize[m*4+3] + f*imgSize[m*4+2];
									
									//printf("----- mm #%d : %dx%d , dxtSize: %d, startAt: %d (%d from SoF)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2],(unsigned int) data,(unsigned int) (data-txtData));
									
									unsigned int txtTypeFS = txtType;
									if(txtTypeFS == GL_TEXTURE_CUBE_MAP)
									{
										switch(f)
										{
											case 0:
												txtTypeFS = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
												break;
											case 1:
												txtTypeFS = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
												break;
											case 2:
												txtTypeFS = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
												break;
											case 3:
												txtTypeFS = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
												break;
											case 4:
												txtTypeFS = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
												break;
											case 5:
												txtTypeFS = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
												break;
										}
									}
									
									if(Scene::enableTextureCompression)
									{
										const GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;
										if(txtHeader->highResImageFormat == IMAGE_FORMAT_DXT1)
										{
											glCompressedTexImage2D(	txtTypeFS, m, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
										}
										else if(txtHeader->highResImageFormat == IMAGE_FORMAT_DXT5)
										{
											glCompressedTexImage2D(	txtTypeFS, m, COMPRESSED_RGBA_S3TC_DXT5_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
										}
									}
									else
									{
										
										// old code -  not used anymore
										
										// sadly, webgl doesn't support dxt5
										// we have to unpack this on the fly :(
										//glCompressedTexImage2D(	GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, imgSize[i*3+1], imgSize[i*3+0], 0, imgSize[i*3+2], data);
									
										// rgba
										int allocSize = imgSize[m*4 + 0] * imgSize[m*4 + 1] * 4;
										
										//if(txtHeader->highResImageFormat == IMAGE_FORMAT_DXT1)
										//	printf("----- mm #%d : %dx%d , dxtSize: %d, rgbaSize: %d (decompressed, dxt1 not supported)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2],allocSize);
										//else
										//	printf("----- mm #%d : %dx%d , dxtSize: %d, rgbaSize: %d (decompressed, dxt5 not supported)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2],allocSize);
										
										unsigned char* unpacked = (unsigned char*) malloc(allocSize);
										
										squish::DecompressImage( unpacked, imgSize[m*4 + 1], imgSize[m*4 + 0], data, sqImageType );
										
										glTexImage2D(txtTypeFS, m, GL_RGBA, imgSize[m*4 + 1], imgSize[m*4 + 0], 0, GL_RGBA, GL_UNSIGNED_BYTE, unpacked);
										
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
								
							}
							
							glBindTexture(GL_TEXTURE_2D, 0);
							
						}
						
					}
				}
				
				printf("Texture %s ready\n",fileName);
			}
		}
	}
}

void Texture::Bind(int i)
{
	if(state == FS_READY)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(txtType, this->textureId);
	}
}
void Texture::Unbind(int i)
{
	if(state == FS_READY)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(txtType, 0);
	}
}