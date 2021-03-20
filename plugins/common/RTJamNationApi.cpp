#include "RTJamNationApi.hpp"

using namespace std;

json resultJson;

void RTJamNationApi::testMe(string url) {
  resultJson = json::parse("{}");
  restincurl::Client client;
  client.Build()->Get(url)
      .Trace()
      .Option(CURLOPT_SSL_VERIFYPEER, 0L)
      .AcceptJson()
      .Header("X-Client", "restincurl")
      .Trace()
      .WithCompletion([](const restincurl::Result& result) {
        clog << result.msg << endl;
        try {
          resultJson = json::parse(result.body);
        } catch (...) {
          clog << "Failed to parse server json" << endl;
        }
        clog << "In callback! HTTP result code was " << result.http_response_code << endl;
        clog << "Data was " << result.body.size() << " bytes." << endl;
        clog << resultJson << endl;
        // m_resultBody = json::parse(result.body);
      })
      .Execute();

  // If the client goes out of scope, it will terminate all ongoing
  // requests, so we need to wait here for the request to finish.

  // Tell client that we want to close when the request is finish
  client.CloseWhenFinished();

  // Wait for the worker-thread in the client to quit
  client.WaitForFinish();
}