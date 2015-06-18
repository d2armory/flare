
#include "fileLoader.hpp"

LoadedFile* FileLoader::loaded = 0;

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
	
	// blank string check
	if(strcmp("",fileName)==0) return;
	// existing file checl
	if(FileLoader::FileExist(fileName)) return;
	
	std::string fName(fileName);
	
	// currently loading file checl
	LoadedFile* l = loaded;
	if(l!=0)
	{
		if(l->fileName.compare(fName)==0)
		{
			return;
		}
		l = l->nextFile;
	}
	
	std::string baseUrl = "http://data-1.mantastyle.com/dota2/";
	//std::string baseUrl = "http://dota2-assets.yearbeast.com/dota2/";
	//std::string baseUrl = "http://104.236.208.106:8080/";
	
	std::string fileUrl = baseUrl + fName;
	
	FileLoader::PrepareDirectory(fileName);
	
	printf("[Loader] Fetching: %s\n",fileUrl.c_str());
	
	LoadedFile* lf = new LoadedFile();
	lf->fileName = fName;
	lf->nextFile = loaded;
	loaded = lf;
	
	emscripten_async_wget2(fileUrl.c_str(),fileName,"GET","",0,&FileLoader::LoadCallbackSuccess,&FileLoader::LoadCallbackFail,0);
	
}

void FileLoader::LoadCallbackSuccess(unsigned int num, void* some, const char* fileName)
{
	printf("[Loader] Fetched: %s\n",fileName);
}

void FileLoader::LoadCallbackFail(unsigned int num, void* some, int num2)
{
	//printf("Fail to load: %s\n",fileName);
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

char* FileLoader::ReadFile(const char* fileName, unsigned int& size)
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
	
	size = fileSize;
	
	return fileData;

}

char* FileLoader::ReadFile(const char* fileName)
{
	unsigned int size = 0;
	return ReadFile(fileName, size);	
}