
#include "fileLoader.hpp"

void FileLoader::PrepareDirectory(const char* fileName)
{
	std::string s(fileName);
	std::string cur = "";

    std::string::size_type prev_pos = 0, pos = 0;
    while( (pos = s.find('/', pos)) != std::string::npos )
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        //std::cout << substring << '\n';
        if(prev_pos!=0)
        {
        	cur = cur + "/";
        }
        cur = cur + substring;
        
        std::string cmd = "try{ FS.mkdir('/" + cur + "'); } catch(err) {}";
        //printf("called: %s\n",cmd.c_str());
        
        emscripten_run_script(cmd.c_str());

        prev_pos = ++pos;
    }
    //std::string substring( s.substr(prev_pos, pos-prev_pos) ); // Last word
    //std::cout << substring << '\n';
}

void FileLoader::Load(const char* fileName)
{
	
	std::string baseUrl = "http://104.236.208.106/dota2/";
	std::string fName(fileName);
	
	std::string fileUrl = baseUrl + fName;
	
	FileLoader::PrepareDirectory(fileName);
	
	printf("Fetching: %s\n",fileUrl.c_str());
	
	emscripten_async_wget(fileUrl.c_str(),fileName,&FileLoader::LoadCallbackSuccess,&FileLoader::LoadCallbackFail);
	
}

void FileLoader::LoadCallbackSuccess(const char* fileName)
{
	printf("Fetched: %s\n",fileName);
}

void FileLoader::LoadCallbackFail(const char* fileName)
{
	printf("Fail to load: %s\n",fileName);
}

bool FileLoader::FileExist(const char* fileName)
{
	FILE *file;
	file = fopen(fileName, "r");
	if (file)
	{
		fclose(file);
		return true;
	}
	return false;
}

char* FileLoader::ReadFile(const char* fileName)
{
	FILE* fp = fopen(fileName,"rb");
	
	if(!fp)
	{
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	char* fileData = (char*) malloc(fileSize);
	int frrt = fread(fileData,fileSize,fileSize,fp);
	
	fclose(fp);
	
	if(frrt==0) {
		//printf("File read failed!\n");
		free(fileData);
		return 0;
	}
	
	return fileData;

}