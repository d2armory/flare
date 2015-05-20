// mdl file information dump

#include <stdio.h>
#include <stdlib.h>
#include "mdl/common.h"
#include "glm/glm.hpp"

int main(void)
{
    
    printf("Validating pointer size\n");
    
    const int pointerSizeExpected = 4;
    const int pointerSize = sizeof(void*);
    
    printf("- Expected: %d, Runtime: %d\n",pointerSizeExpected,pointerSize);
    
    if(pointerSize!=pointerSizeExpected) return 1;
    
    printf("Validating mdlHeader size\n");
    
    const int mdlHeaderSizeExpected = 408;
    const int mdlHeaderSize = sizeof(mdlHeader);
    
    printf("- Expected: %d, Runtime: %d\n",mdlHeaderSizeExpected,mdlHeaderSize);
    
    if(mdlHeaderSize!=mdlHeaderSizeExpected) return 1;
    
    printf("Openning mdl file\n");
    
    const char* fileName = "testbin/axe.mdl";
    
    FILE* fp = fopen(fileName,"rb");
    fseek(fp, 0, SEEK_END);
    int fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    printf("File size: %d bytes\n",fileSize);
    
    char* fileData = (char*) malloc(fileSize);
    int frrt = fread(fileData,fileSize,fileSize,fp);
    
    if(frrt==0) {
        printf("File read failed!\n");
        return 1;
    }
    
    printf("File data read into memory (0x%X)\n",(unsigned int) fileData);
    
    fclose(fp);
    
    printf("Apply header on data\n");
    
    mdlHeader* mh = (mdlHeader*) fileData;
    
    char* mhId = (char*) &mh->id;
    
    printf("- ID: %c%c%c%c (0x%X)\n",*mhId,*(mhId+1),*(mhId+2),*(mhId+3),mh->id);
    printf("- Version: %d\n",mh->version);
    printf("- Checksum: 0x%X\n",mh->checksum);
    printf("- Name: %s\n",mh->name);
    printf("- Length: %d\n",mh->length);
    
    
    const bool dumpBones = true;
    const bool dumpAnim = true;
    const bool dumpTextures = true;
    
    
    if(dumpBones)
    {
        printf("Retrieving bone data\n");
        
        printf("- Num bones: %d\n",mh->numbones);
        
        for(int i=0;i<mh->numbones;i++)
        {
            mdlBone* bone = mh->pBone(i);
            printf("--- %i: %s",i,bone->pszName());
            if(bone->parent != -1)
            {
                printf(" - %s (%i)",mh->pBone(bone->parent)->pszName(),bone->parent);
            }
            printf("\n");
        }
    }
    
    if(dumpTextures)
    {
        printf("Retrieving texture list\n");
        
        printf("- Num textures: %d\n",mh->numtextures);
        
        for(int i=0;i<mh->numtextures;i++)
        {
            mdlTexture* texture = mh->pTexture(i);
            printf("--- %i: %s\n",i,texture->pszName());
        }
    }
    
    free(fileData);
    
    return 0;
}