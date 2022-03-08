#define NOMINMAX

#include "arguments/Arguments.hpp"
#include "client/restclient.h"
#include "client/connection.h"
#include "unsuck.hpp"
#include "threadpool.hpp"

#include <iostream>

using namespace RestClient;
using namespace std;

int main(int argc, char **argv)
{
    Arguments args(argc, argv);
    args.addArgument("help,h", "Display help information");
    args.addArgument("outdir,o", "output directory");
    args.addArgument("cookie,c", "user login cookie");
    args.addArgument("start_lon", "[-180,180]");
    args.addArgument("end_lon", "[-180,180]");
    args.addArgument("start_lat", "[-90,90]");
    args.addArgument("end_lat", "[-90,90]");
    args.addArgument("proxy", "http proxy");
    args.addArgument("threads_num", "threads num");

    if (args.has("help"))
    {
        cout << endl
             << args.usage() << endl;
        exit(0);
    }

    string outdir = args.get("outdir").as<string>("G:/GDEM");
    fs::create_directories(outdir);

    string cookie = args.get("cookie").as<string>("_ga=GA1.2.692002381.1646191984; _ga_XXXXXXXXXX=GS1.1.1646192430513.6kkqcg1g.1.1.1646192430.0; DATA=Yia_yuVpZdOYKMzbScaNogAAATw");
    int start_lon = args.get("start_lon").as<int>(72);
    int end_lon = args.get("end_lon").as<int>(135);
    int start_lat = args.get("start_lat").as<int>(18);
    int end_lat = args.get("end_lat").as<int>(53);
    string proxy = args.get("proxy").as<string>();
    int threads_num = args.get("threads_num").as<int>(1);

    string base_url = "https://e4ftl01.cr.usgs.gov/ASTT/ASTGTM.003/2000.03.01/ASTGTMV003_";

    // begin
    RestClient::init();

    Connection conn("");
    conn.SetVerifyPeer(false);
    conn.FollowRedirects(true);
    conn.AppendHeader("Cookie", cookie);
    conn.SetProxy(proxy);

    struct Task
    {
        int lon;
        int lat;
    };

    std::queue<Task> tasks;
    mutex tasks_mutex;
    bool completed = false;

    progschj::ThreadPool pool(threads_num);
    for (int i = 0; i < threads_num; i++)
    {
        pool.enqueue(
            [&]()
            {
                while (true)
                {
                    tasks_mutex.lock();
                    if (!tasks.empty())
                    {
                        auto task = tasks.front();
                        tasks.pop();
                        tasks_mutex.unlock();

                        int lon = task.lon;
                        int lat = task.lat;
                        string url = base_url;
                        if (lat < 0)
                        {
                            lat = -lat;
                            url += "S";
                        }
                        else
                        {
                            url += "N";
                        }
                        string lat_str = leftPad(formatNumber(lat), 2, '0');
                        url += lat_str;

                        if (lon < 0)
                        {
                            lon = -lon;
                            url += "W";
                        }
                        else
                        {
                            url += "E";
                        }
                        string lon_str = leftPad(formatNumber(lon), 3, '0');
                        url += lon_str;

                        url += ".zip";

                        string file_name = fs::path(url).filename().string();
                        string file_path = outdir + "/" + file_name;
                        if (fs::exists(file_path))
                        {
                            cout << file_name << " already exist." << endl;
                            continue;
                        }

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
                    else
                    {
                        tasks_mutex.unlock();
                        if (completed)
                        {
                            return;
                        }
                    }
                }
            });
    }

    for (int lat = start_lat; lat <= end_lat; lat++)
    {
        for (int lon = start_lon; lon <= end_lon; lon++)
        {
            tasks_mutex.lock();
            if (tasks.size() > 100)
            {
                tasks_mutex.unlock();
                ::Sleep(1000);
            }
            else
            {
                Task task;
                task.lon = lon;
                task.lat = lat;
                tasks.push(task);
                tasks_mutex.unlock();
            }
        }
    }

    completed = true;
    pool.wait_until_nothing_in_flight();
    pool.wait_until_empty();

    RestClient::disable();

    cout << "download completed." << endl;
    getchar();
    return 0;
}