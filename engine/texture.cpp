
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
			
			isCubemap = ((txtHeader->flags & 0x4000) != 0);
			
			txtType = GL_TEXTURE_2D;
			if(isCubemap) txtType = GL_TEXTURE_CUBE_MAP;
			
			int numFaces = (isCubemap)?6:1;
			
			char dxtsupport = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"WEBGL_compressed_texture_s3tc");
			
			if(dxtsupport) printf("- using webgl compressed texture\n");
			
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
							offset += imgSize[j*4 + 2] * numFaces;
						}
						
						for(int m=0;m<txtHeader->mipmapCount;m++)
						{
							for(int f=0;f<numFaces;f++)
							{
								char* data = entry + imgSize[m*4+3] + f*imgSize[m*4+2];
								
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
								
								if(dxtsupport && txtHeader->highResImageFormat == IMAGE_FORMAT_DXT1)
								{
									glCompressedTexImage2D(	txtTypeFS, m, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
								
									//printf("----- mm #%d : %dx%d , dxtSize: %d (webgl dxt1)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2]);
									
								}
								else if(dxtsupport && txtHeader->highResImageFormat == IMAGE_FORMAT_DXT5)
								{
									
									const GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

									glCompressedTexImage2D(	txtTypeFS, m, COMPRESSED_RGBA_S3TC_DXT5_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
								
									//printf("----- mm #%d : %dx%d , dxtSize: %d (webgl dxt5 extension)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2]);
									
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