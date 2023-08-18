#include "http_server.hh"

#include <vector>

#include <sys/stat.h>

#include <fstream>
#include <sstream>

vector<string> split(const string &s, char delim)
{
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim))
  {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request)
{
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  /*
   TODO : extract the request method and URL from first_line here
  */
  this->method = first_line[0];
  this->url = first_line[1];
  if (this->method != "GET")
  {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req)
{
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;

  response->HTTP_version = "1.0";

  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body;
    string url_check;
    {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
      url_check = url + string("/index.html");
      if (stat(url_check.c_str(), &sb) == 0)
        url = url_check;
      else
      {
        response->status_code = "404";
        response->status_text = "Not Found";
        response->content_type = "text/html";
        response->body = "File not found";
        response->content_length = to_string(response->body.length());
        /*
        TODO : set the remaining fields of response appropriately
        */
      }
    }

    /*
    TODO : open the file and read its contents
    */
    fstream reader;
    reader.open(url);
    string output, buff;
    while (getline(reader, buff))
    {
      output = output + buff;
      output = output + string("\n");
    }
    reader.close();
    response->body = output;
    /*
    TODO : set the remaining fields of response appropriately
    */
    response->content_length = to_string(output.length());
  }

  else
  {
    response->status_code = "404";
    response->status_text = "Not Found";
    response->content_type = "text/html";
    response->body = "<html><h1>File not found</html>";
    response->content_length = to_string(response->body.length());
    /*
    TODO : set the remaining fields of response appropriately
    */
  }
  // Invoke get_string() on the response and return
  delete request;

  return response;
  // delete response;
}

string HTTP_Response::get_string()
{
  /*
  TODO : implement this function
  */
  // Used the format given in tutorial point link
  string result = "";
  result += ("HTTP/" + this->HTTP_version +" " + this->status_code + " "+ this->status_text + "\n");
  result += ("Content-Length: " + this->content_length + "\n");
  result += ("Content-Type: " + this->content_type + "\n" + "\n");
  result += this->body;
  return result;
}
