#define NOMINMAX

#include "arguments/Arguments.hpp"
#include "client/restclient.h"
#include "client/connection.h"
#include "unsuck.hpp"

#include <iostream>

using namespace RestClient;
using namespace std;

int main(int argc, char **argv)
{
    Arguments args(argc, argv);
    args.addArgument("outdir,o", "output directory");

    string outdir = args.get("outdir").as<string>("G:/GDEM");
    fs::create_directories(outdir);

    // begin
    RestClient::init();

    string url_template = "https://e4ftl01.cr.usgs.gov/ASTT/ASTGTM.003/2000.03.01/ASTGTMV003_N{lat}E{lon}.zip";

    for (int lat = 18; lat <= 53; lat++)
    {
        for (int lon = 72; lon <= 135; lon++)
        {
            string lon_str = leftPad(formatNumber(lon), 3, '0');
            string lat_str = leftPad(formatNumber(lat), 2, '0');
            string url = stringReplace(url_template, "{lon}", lon_str);
            url = stringReplace(url, "{lat}", lat_str);

            string file_name = fs::path(url).filename().string();
            string file_path = outdir + "/" + file_name;
            if (fs::exists(file_path))
            {
                cout << file_name << " already exist." << endl;
                continue;
            }

            Connection conn("");
            conn.SetVerifyPeer(false);
            conn.FollowRedirects(true);
            conn.AppendHeader("Cookie", "_ga=GA1.2.692002381.1646191984; _ga_XXXXXXXXXX=GS1.1.1646192430513.6kkqcg1g.1.1.1646192430.0; DATA=Yia_yuVpZdOYKMzbScaNogAAATw");
            auto response = conn.get(url);
            if (response.code >= 200 && response.code < 300)
            {
                writeBinaryFile(outdir + "/" + file_name, response.body);
                cout << file_name << " download succeeded." << endl;
            }
            else
            {
                cout << "error: " << response.code << ", " << file_name << endl;
            }
        }
    }

    RestClient::disable();

    getchar();
    return 0;
}