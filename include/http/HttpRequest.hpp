#pragma once 
# include <string>





class HttpRequest {
	public:
		std::string method;
		std::string path;
		std::string version;


		void print() const;
};
