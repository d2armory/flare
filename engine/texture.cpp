
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
	isVtex = false;
	txtType = GL_TEXTURE_2D;
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
			
			printf("texture [S2]: %s\n",fileName);
			
			// SOURCE 2 format
			isVtex = true;
			
			// TODO: use new kvreader2 instead
			
			dmxHeader* dmxH = (dmxHeader*) txtData;
			vtexHeader* vtexH = 0;
			unsigned int vtexHeaderSize = 0;
			// Finding data block
			for(int i=0;i<dmxH->blockCount;i++)
			{
				dmxBlockHeader* blockHeader = dmxH->block(i);
				if(blockHeader->name[0] == 'D')
				{
					vtexH = blockHeader->vtexData();
					vtexHeaderSize = blockHeader->dataSize;
				}
			}
			if(vtexH == 0)
			{
				printf("Error locating data block!\n");
				return;
			}
			
			/* for(int x=0;x<52;x++)
			{
				printf("%X ",*(((unsigned char*)(vtexH)) + x));
			}
			printf("\n"); */
			
			printf("- size: WxH =  %dx%d\n",vtexH->width,vtexH->height);
			printf("- mipmap count: %d\n",vtexH->mipLevel);
			printf("- format: %d\n",vtexH->format);
			
			//return;
			
			// variable preparation
			isCubemap = false; // TODO: accept cubemap
			txtType = GL_TEXTURE_2D;
			if(isCubemap) txtType = GL_TEXTURE_CUBE_MAP;
			int numFaces = (isCubemap)?6:1;
			unsigned int sqImageType = squish::kDxt1;	// format in squish type
			switch(vtexH->format)
			{
				case 1://IMAGE_FORMAT_DXT1:
					sqImageType = squish::kDxt1;
					break;
				case 2://IMAGE_FORMAT_DXT5:
					sqImageType = squish::kDxt5;
					break;
				default:
					sqImageType = 1 << 31;
					printf("----- unsupported image format %d\n",vtexH->format);
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
			int numMip = vtexH->mipLevel;
			// TODO: use struct for this (4x array so ugly)
			int* imgSize = new int[numMip * 4];
			
			// Calculate width, height, size in byte first
			int tmpH = vtexH->height;
			int tmpW = vtexH->width;
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
			char* entry = ((char*) txtData) + dmxH->fileSize;
			for(int m=0;m<numMip;m++)
			{
				for(int f=0;f<numFaces;f++)
				{
					// get data pointer
					char* data = entry + imgSize[m*4+3];
					printf("----- mm #%d : %dx%d , dxtSize: %d, startAt: %d (%d from SoF, %d from entry)\n",m,imgSize[m*4 + 1],imgSize[m*4 + 0],imgSize[m*4 + 2],(unsigned int) data,(unsigned int) (data-txtData),imgSize[m*4+3]);
					
					/* if(m==numMip-1)
					{
						for(int b=0;b<imgSize[m*4+2];b++) printf("%X ",(unsigned char) *(data+b));
					
						printf("\n");
					} */
					
					
					unsigned int txtTypeFS = txtType;
					
					// Seem to can't get Vtex DXT to work with webgl
					// don't know why so disable for time being
					if(false && Scene::enableTextureCompression)
					{
						// Use compressed texture if possible
						const GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;
						if(vtexH->format == 1)//IMAGE_FORMAT_DXT1)
						{
							glCompressedTexImage2D(	txtTypeFS, m, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, imgSize[m*4+1], imgSize[m*4+0], 0, imgSize[m*4+2], data);
						}
						else if(vtexH->format == 2)//IMAGE_FORMAT_DXT5)
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
						
						/* if(m==numMip-3)
						{
							for(int b=0;b<allocSize;b+=4) 
							{
								if(b % (imgSize[m*4 + 1]*4) == 0) printf("<br />");
								printf("<span style=\"background: rgba(%d,%d,%d,%f);\">&nbsp;&nbsp;&nbsp;</span>",*(unpacked+b),*(unpacked+b+1),*(unpacked+b+2),*(unpacked+b+3)/255.0);
							}
							printf("\n");
						} */
						
						free(unpacked);
					}
				}
				
			}
			
			// for some reason, vtex seem not to provide all mipmap level
			glGenerateMipmap(GL_TEXTURE_2D);
			
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void Texture::Bind(int i)
{
	if(state == FS_READY && this->textureId != 0)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(txtType, this->textureId);
	}
}
void Texture::Unbind(int i)
{
	if(state == FS_READY && this->textureId != 0)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(txtType, 0);
	}
}