#include <fstream>
#include <algorithm>

#include <rapidjson/document.h>

#include "misc.h"
#include "printout.h"
#include "logger.h"
#include "speedtestutil.h"
#include "webget.h"
#include "rapidjson_extra.h"
#include "ini_reader.h"

using namespace rapidjson;
using namespace YAML;

std::string base_ss_win = "{\"version\":\"4.1.6\",\"configs\":[?config?],\"strategy\":null,\"index\":0,\"global\":false,\"enabled\":false,\"shareOverLan\":true,\"isDefault\":false,\"localPort\":?localport?,\"portableMode\":true,\"pacUrl\":null,\"useOnlinePac\":false,\"secureLocalPac\":true,\"availabilityStatistics\":false,\"autoCheckUpdate\":true,\"checkPreRelease\":false,\"isVerboseLogging\":false,\"logViewer\":{\"topMost\":false,\"wrapText\":false,\"toolbarShown\":false,\"Font\":\"Consolas, 8pt\",\"BackgroundColor\":\"Black\",\"TextColor\":\"White\"},\"proxy\":{\"useProxy\":false,\"proxyType\":0,\"proxyServer\":\"\",\"proxyPort\":0,\"proxyTimeout\":3,\"useAuth\":false,\"authUser\":\"\",\"authPwd\":\"\"},\"hotkey\":{\"SwitchSystemProxy\":\"\",\"SwitchSystemProxyMode\":\"\",\"SwitchAllowLan\":\"\",\"ShowLogs\":\"\",\"ServerMoveUp\":\"\",\"ServerMoveDown\":\"\",\"RegHotkeysAtStartup\":false}}";
std::string config_ss_win = "{\"server\":\"?server?\",\"server_port\":?port?,\"password\":\"?password?\",\"method\":\"?method?\",\"plugin\":\"./?plugin?\",\"plugin_opts\":\"?plugin_opts?\",\"plugin_args\":\"\",\"remarks\":\"?remarks?\",\"timeout\":5}";
std::string config_ss_libev = "{\"server\":\"?server?\",\"server_port\":?port?,\"password\":\"?password?\",\"method\":\"?method?\",\"plugin\":\"?plugin?\",\"plugin_opts\":\"?plugin_opts?\",\"plugin_args\":\"\",\"local_address\":\"127.0.0.1\",\"local_port\":?localport?}";
std::string base_ssr_win = "{\"configs\":[?config?],\"index\":0,\"random\":true,\"sysProxyMode\":1,\"shareOverLan\":false,\"localPort\":?localport?,\"localAuthPassword\":null,\"localDnsServer\":\"\",\"dnsServer\":\"\",\"reconnectTimes\":2,\"balanceAlgorithm\":\"LowException\",\"randomInGroup\":false,\"TTL\":0,\"connectTimeout\":5,\"proxyRuleMode\":2,\"proxyEnable\":false,\"pacDirectGoProxy\":false,\"proxyType\":0,\"proxyHost\":null,\"proxyPort\":0,\"proxyAuthUser\":null,\"proxyAuthPass\":null,\"proxyUserAgent\":null,\"authUser\":null,\"authPass\":null,\"autoBan\":false,\"checkSwitchAutoCloseAll\":false,\"logEnable\":false,\"sameHostForSameTarget\":false,\"keepVisitTime\":180,\"isHideTips\":false,\"nodeFeedAutoUpdate\":true,\"serverSubscribes\":[],\"token\":{},\"portMap\":{}}";
std::string config_ssr_win = "{\"remarks\":\"?remarks?\",\"id\":\"18C4949EBCFE46687AE4A7645725D35F\",\"server\":\"?server?\",\"server_port\":?port?,\"server_udp_port\":0,\"password\":\"?password?\",\"method\":\"?method?\",\"protocol\":\"?protocol?\",\"protocolparam\":\"?protoparam?\",\"obfs\":\"?obfs?\",\"obfsparam\":\"?obfsparam?\",\"remarks_base64\":\"?remarks_base64?\",\"group\":\"?group?\",\"enable\":true,\"udp_over_tcp\":false}";
std::string config_ssr_libev = "{\"server\":\"?server?\",\"server_port\":?port?,\"protocol\":\"?protocol?\",\"method\":\"?method?\",\"obfs\":\"?obfs?\",\"password\":\"?password?\",\"obfs_param\":\"?obfsparam?\",\"protocol_param\":\"?protoparam?\",\"local_address\":\"127.0.0.1\",\"local_port\":?localport?}";
std::string base_vmess = "{\"inbounds\":[{\"port\":?localport?,\"listen\":\"127.0.0.1\",\"protocol\":\"socks\"}],\"outbounds\":[{\"tag\":\"proxy\",\"protocol\":\"vmess\",\"settings\":{\"vnext\":[{\"address\":\"?add?\",\"port\":?port?,\"users\":[{\"id\":\"?id?\",\"alterId\":?aid?,\"email\":\"t@t.tt\",\"security\":\"?cipher?\"}]}]},\"streamSettings\":{\"network\":\"?net?\",\"security\":\"?tls?\",\"tlsSettings\":?tlsset?,\"tcpSettings\":?tcpset?,\"wsSettings\":?wsset?},\"mux\":{\"enabled\":true}}],\"routing\":{\"domainStrategy\":\"IPIfNonMatch\"}}";
std::string wsset_vmess = "{\"connectionReuse\":true,\"path\":\"?path?\",\"headers\":{\"Host\":\"?host?\"?edge?}}";
std::string tcpset_vmess = "{\"connectionReuse\":true,\"header\":{\"type\":\"?type?\",\"request\":{\"version\":\"1.1\",\"method\":\"GET\",\"path\":[\"?path?\"],\"headers\":{\"Host\":[\"?host?\"],\"User-Agent\":[\"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.75 Safari/537.36\",\"Mozilla/5.0 (iPhone; CPU iPhone OS 10_0_2 like Mac OS X) AppleWebKit/601.1 (KHTML, like Gecko) CriOS/53.0.2785.109 Mobile/14A456 Safari/601.1.46\"],\"Accept-Encoding\":[\"gzip, deflate\"],\"Connection\":[\"keep-alive\"],\"Pragma\":\"no-cache\"}}}}";
std::string tlsset_vmess = "{\"serverName\":\"?serverName?\",\"allowInsecure\":false,\"allowInsecureCiphers\":false}";

string_array ss_ciphers = {"rc4-md5", "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "aes-128-ctr", "aes-192-ctr", "aes-256-ctr", "camellia-128-cfb", "camellia-192-cfb", "camellia-256-cfb", "bf-cfb", "chacha20-ietf-poly1305", "xchacha20-ietf-poly1305", "salsa20", "chacha20", "chacha20-ietf"};
string_array ssr_ciphers = {"none", "table", "rc4", "rc4-md5", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "aes-128-ctr", "aes-192-ctr", "aes-256-ctr", "bf-cfb", "camellia-128-cfb", "camellia-192-cfb", "camellia-256-cfb", "cast5-cfb", "des-cfb", "idea-cfb", "rc2-cfb", "seed-cfb", "salsa20", "chacha20", "chacha20-ietf"};

std::map<std::string, std::string> parsedMD5;
std::string modSSMD5 = "f7653207090ce3389115e9c88541afe0";

//remake from speedtestutil

std::string vmessConstruct(std::string add, std::string port, std::string type, std::string id, std::string aid, std::string net, std::string cipher, std::string path, std::string host, std::string edge, std::string tls, int local_port)
{
    std::string base = base_vmess;
    std::string wsset = wsset_vmess;
    std::string tcpset = tcpset_vmess;
    std::string tlsset = tlsset_vmess;
    host = trim(host);
    path = trim(path);
    base = replace_all_distinct(base, "?localport?", std::to_string(local_port));
    base = replace_all_distinct(base, "?add?", add);
    base = replace_all_distinct(base, "?port?", port);
    base = replace_all_distinct(base, "?id?", id);
    base = replace_all_distinct(base, "?aid?", aid);
    base = replace_all_distinct(base, "?net?", net);
    base = replace_all_distinct(base, "?cipher?", cipher);
    if(net == "ws")
    {
        wsset = replace_all_distinct(wsset, "?host?", host.empty() ? add : host);
        wsset = replace_all_distinct(wsset, "?path?", path.empty() ? "/" : path);
        wsset = replace_all_distinct(wsset, "?edge?", edge.empty() ? "" : ",\"Edge\":\"" + edge + "\"");
        base = replace_all_distinct(base, "?wsset?", wsset);
    }
    if(type == "http")
    {
        tcpset = replace_all_distinct(tcpset, "?host?", host.empty() ? add : host);
        tcpset = replace_all_distinct(tcpset, "?type?", type);
        tcpset = replace_all_distinct(tcpset, "?path?", path.empty() ? "/" : path);
        base = replace_all_distinct(base, "?tcpset?", tcpset);
    }
    if(host.empty() && !isIPv4(add) && !isIPv6(add))
        host = add;
    if(host != "")
    {
        tlsset = replace_all_distinct(tlsset, "?serverName?", host);
        base = replace_all_distinct(base, "?tlsset?", tlsset);
    }

    base = replace_all_distinct(base, "?tls?", tls);
    base = replace_all_distinct(base, "?tcpset?", "null");
    base = replace_all_distinct(base, "?wsset?", "null");
    base = replace_all_distinct(base, "?tlsset?", "null");

    return base;
}

std::string ssrConstruct(std::string group, std::string remarks, std::string remarks_base64, std::string server, std::string port, std::string protocol, std::string method, std::string obfs, std::string password, std::string obfsparam, std::string protoparam, int local_port, bool libev)
{
    std::string base = base_ssr_win;
    std::string config = config_ssr_win;
    std::string config_libev = config_ssr_libev;
    if(libev == true)
        config = config_libev;
    if(isIPv6(server))
        server = "[" + server + "]";
    config = replace_all_distinct(config, "?group?", group);
    config = replace_all_distinct(config, "?remarks?", remarks);
    config = replace_all_distinct(config, "?remarks_base64?", remarks_base64);
    config = replace_all_distinct(config, "?server?", server);
    config = replace_all_distinct(config, "?port?", port);
    config = replace_all_distinct(config, "?protocol?", protocol);
    config = replace_all_distinct(config, "?method?", method);
    config = replace_all_distinct(config, "?obfs?", obfs);
    config = replace_all_distinct(config, "?password?", password);
    config = replace_all_distinct(config, "?obfsparam?", obfsparam);
    config = replace_all_distinct(config, "?protoparam?", protoparam);
    if(libev == true)
        base = config;
    else
        base = replace_all_distinct(base, "?config?", config);
    base = replace_all_distinct(base, "?localport?", std::to_string(local_port));

    return base;
}

std::string ssConstruct(std::string server, std::string port, std::string password, std::string method, std::string plugin, std::string pluginopts, std::string remarks, int local_port, bool libev)
{
    std::string base = base_ss_win;
    std::string config = config_ss_win;
    std::string config_libev = config_ss_libev;
    if(plugin == "obfs-local")
        plugin = "simple-obfs";
    if(plugin.size())
        plugin = "./" + plugin;
    if(libev == true)
        config = config_libev;
    if(isIPv6(server))
        server = "[" + server + "]";
    config = replace_all_distinct(config, "?server?", server);
    config = replace_all_distinct(config, "?port?", port);
    config = replace_all_distinct(config, "?password?", password);
    config = replace_all_distinct(config, "?method?", method);
    config = replace_all_distinct(config, "?plugin?", plugin);
    config = replace_all_distinct(config, "?plugin_opts?", pluginopts);
    config = replace_all_distinct(config, "?remarks?", remarks);
    if(libev == true)
        base = config;
    else
        base = replace_all_distinct(base, "?config?", config);
    base = replace_all_distinct(base, "?localport?", std::to_string(local_port));

    return base;
}

std::string socksConstruct(std::string remarks, std::string server, std::string port, std::string username, std::string password)
{
    return "user=" + username + "&pass=" + password;
}

std::string httpConstruct(std::string remarks, std::string server, std::string port, std::string username, std::string password, bool tls)
{
    return "user=" + username + "&pass=" + password;
}

int explodeLog(std::string log, std::vector<nodeInfo> &nodes)
{
    INIReader ini;
    std::vector<std::string> nodeList, vArray;
    std::string strTemp;
    nodeInfo node;
    ini.Parse(log);

    if(!ini.SectionExist("Basic"))
        return -1;

    nodeList = ini.GetSections();
    node.proxyStr = "LOG";
    for(auto &x : nodeList)
    {
        if(x == "Basic")
            continue;
        ini.EnterSection(x);
        vArray = split(x, "^");
        node.group = vArray[0];
        node.remarks = vArray[1];
        node.avgPing = ini.Get("AvgPing");
        node.avgSpeed = ini.Get("AvgSpeed");
        node.groupID = ini.GetInt("GroupID");
        node.id = ini.GetInt("ID");
        node.maxSpeed = ini.Get("MaxSpeed");
        node.online = ini.GetBool("Online");
        node.pkLoss = ini.Get("PkLoss");
        ini.GetIntArray("RawPing", ",", node.rawPing);
        ini.GetIntArray("RawSitePing", ",", node.rawSitePing);
        ini.GetIntArray("RawSpeed", ",", node.rawSpeed);
        node.sitePing = ini.Get("SitePing");
        node.totalRecvBytes = ini.GetInt("UsedTraffic");
        node.ulSpeed = ini.Get("ULSpeed");
        nodes.push_back(node);
    }

    return 0;
}

template <typename T> void operator >> (const YAML::Node& node, T& i)
{
    if(node.IsDefined() && !node.IsNull()) //fail-safe
        i = node.as<T>();
};

template <typename T> T safe_as (const YAML::Node& node)
{
    if(node.IsDefined() && !node.IsNull())
        return node.as<T>();
    return T();
};

void explodeVmess(std::string vmess, std::string custom_port, int local_port, nodeInfo &node)
{
    std::string version, ps, add, port, type, id, aid, net, path, host, tls;
    Document jsondata;
    std::vector<std::string> vArray;
    if(regMatch(vmess, "vmess://(.*?)\\?(.*)")) //shadowrocket style link
    {
        explodeShadowrocket(vmess, custom_port, local_port, node);
        return;
    }
    else if(regMatch(vmess, "vmess1://(.*?)\\?(.*)")) //kitsunebi style link
    {
        explodeKitsunebi(vmess, custom_port, local_port, node);
        return;
    }
    vmess = urlsafe_base64_decode(regReplace(vmess, "(vmess|vmess1)://", ""));
    if(regMatch(vmess, "(.*?) = (.*)"))
    {
        explodeQuan(vmess, custom_port, local_port, node);
        return;
    }
    jsondata.Parse(vmess.data());
    if(jsondata.HasParseError())
        return;

    version = "1"; //link without version will treat as version 1
    GetMember(jsondata, "v", version); //try to get version

    GetMember(jsondata, "ps", ps);
    GetMember(jsondata, "add", add);
    GetMember(jsondata, "type", type);
    GetMember(jsondata, "id", id);
    GetMember(jsondata, "aid", aid);
    GetMember(jsondata, "net", net);
    GetMember(jsondata, "tls", tls);
    if(custom_port.size())
        port = custom_port;
    else
        GetMember(jsondata, "port", port);

    GetMember(jsondata, "host", host);
    if(version == "1")
    {
        if(host != "")
        {
            vArray = split(host, ";");
            if(vArray.size() == 2)
            {
                host = vArray[0];
                path = vArray[1];
            }
        }
    }
    else if(version == "2")
    {
        path = GetMember(jsondata, "path");
    }

    node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
    node.group = V2RAY_DEFAULT_GROUP;
    node.remarks = ps;
    node.server = add;
    node.port = to_int(port, 0);
    node.proxyStr = vmessConstruct(add, port, type, id, aid, net, "auto", path, host, "", tls, local_port);
}

void explodeVmessConf(std::string content, std::string custom_port, int local_port, bool libev, std::vector<nodeInfo> &nodes)
{
    nodeInfo node;
    Document json;
    std::string group, ps, add, port, type, id, aid, net, path, host, tls, cipher, subid;
    int configType, index = nodes.size();
    std::map<std::string, std::string> subdata;
    std::map<std::string, std::string>::iterator iter;

    json.Parse(content.data());
    if(json.HasParseError())
        return;
    if(json.HasMember("outbounds")) //single config
    {
        if(json["outbounds"].Size() > 0 && json["outbounds"][0].HasMember("settings") && json["outbounds"][0]["settings"].HasMember("vnext") && json["outbounds"][0]["settings"]["vnext"].Size() > 0)
        {
            add = GetMember(json["outbounds"][0]["settings"]["vnext"][0], "address");
            port = GetMember(json["outbounds"][0]["settings"]["vnext"][0], "port");
            json["routing"].RemoveAllMembers();
            json["routing"].AddMember("domainStrategy", "IPIfNonMatch", json.GetAllocator());
            rapidjson::Value& inbounds = json["inbounds"];
            rapidjson::Document newinbound(Type::kObjectType);
            newinbound.AddMember("listen", "127.0.0.1", json.GetAllocator());
            newinbound.AddMember("port", local_port, json.GetAllocator());
            newinbound.AddMember("protocol", "socks", json.GetAllocator());
            inbounds.Clear();
            inbounds.PushBack(newinbound, json.GetAllocator());
            node.proxyStr = SerializeObject(json);
            node.group = V2RAY_DEFAULT_GROUP;
            node.remarks = add + ":" + port;
            node.server = add;
            node.port = to_int(port);
            nodes.push_back(node);
        }
        return;
    }
    //read all subscribe remark as group name
    for(unsigned int i = 0; i < json["subItem"].Size(); i++)
    {
        subdata.insert(std::pair<std::string, std::string>(json["subItem"][i]["id"].GetString(), json["subItem"][i]["remarks"].GetString()));
    }

    for(unsigned int i = 0; i < json["vmess"].Size(); i++)
    {
        if(json["vmess"][i]["address"].IsNull() || json["vmess"][i]["port"].IsNull() || json["vmess"][i]["id"].IsNull())
        {
            continue;
        }
        //common info
        json["vmess"][i]["remarks"] >> ps;
        json["vmess"][i]["address"] >> add;
        if(custom_port.size())
            port = custom_port;
        else
            json["vmess"][i]["port"] >>port;
        json["vmess"][i]["subid"] >> subid;

        if(subid != "")
        {
            iter = subdata.find(subid);
            if(iter != subdata.end())
            {
                group = iter->second;
            }
        }
        if(ps.empty())
        {
            ps = add + ":" + port;
        }

        json["vmess"][i]["configType"] >> configType;
        switch(configType)
        {
        case 1: //vmess config
            json["vmess"][i]["headerType"] >> type;
            json["vmess"][i]["id"] >> id;
            json["vmess"][i]["alterId"] >> aid;
            json["vmess"][i]["network"] >> net;
            json["vmess"][i]["path"] >> path;
            json["vmess"][i]["requestHost"] >> host;
            json["vmess"][i]["streamSecurity"] >> tls;
            json["vmess"][i]["security"] >> cipher;
            group = V2RAY_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
            node.proxyStr = vmessConstruct(add, port, type, id, aid, net, cipher, path, host, "", tls, local_port);
            break;
        case 3: //ss config
            json["vmess"][i]["id"] >> id;
            json["vmess"][i]["security"] >> cipher;
            group = SS_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
            node.proxyStr = ssConstruct(add, port, id, cipher, "", "", ps, local_port, libev);
            break;
        case 4: //socks config
            group = SOCKS_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.proxyStr = socksConstruct(ps, add, port, "", "");
            break;
        default:
            continue;
        }

        node.group = group;
        node.remarks = ps;
        node.id = index;
        node.server = add;
        node.port = to_int(port);
        nodes.push_back(node);
    }
    return;
}

void explodeSS(std::string ss, bool libev, std::string custom_port, int local_port, nodeInfo &node)
{
    std::string ps, password, method, server, port, plugins, plugin, pluginopts, addition, group = SS_DEFAULT_GROUP;
    std::vector<std::string> args, secret;
    ss = replace_all_distinct(replace_all_distinct(ss.substr(5), "\r", ""), "/?", "?");
    if(strFind(ss, "#"))
    {
        ps = UrlDecode(ss.substr(ss.find("#") + 1));
        ss = ss.substr(0, ss.find("#"));
    }

    if(strFind(ss, "?"))
    {
        addition = ss.substr(ss.find("?") + 1);
        plugins = UrlDecode(getUrlArg(addition, "plugin"));
        plugin = plugins.substr(0, plugins.find(";"));
        pluginopts = plugins.substr(plugins.find(";") + 1);
        if(getUrlArg(addition, "group") != "")
            group = urlsafe_base64_decode(getUrlArg(addition, "group"));
        ss = ss.substr(0, ss.find("?"));
    }
    if(strFind(ss, "@"))
    {
        ss = regReplace(ss, "(.*?)@(.*):(.*)", "$1|$2|$3");
        args = split(ss, "|");
        secret = split(urlsafe_base64_decode(args[0]), ":");
        if(args.size() < 3 || secret.size() < 2)
            return;
        method = secret[0];
        password = secret[1];
        server = args[1];
        port = custom_port.empty() ? args[2] : custom_port;
    }
    else
    {
        if(!regMatch(urlsafe_base64_decode(ss), "(.*?):(.*?)@(.*):(.*)"))
            return;
        ss = regReplace(urlsafe_base64_decode(ss), "(.*?):(.*?)@(.*):(.*)", "$1|$2|$3|$4");
        args = split(ss, "|");
        if(args.size() < 4)
            return;
        method = args[0];
        password = args[1];
        server = args[2];
        port = custom_port.empty() ? args[3] : custom_port;
    }
    if(ps.empty())
        ps = server + ":" + port;

    node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
    node.group = group;
    node.remarks = ps;
    node.server = server;
    node.port = to_int(port, 0);
    node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, ps, local_port, libev);
}

void explodeSSD(std::string link, bool libev, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes)
{
    Document jsondata;
    nodeInfo node;
    unsigned int index = nodes.size();
    std::string group, port, method, password, server, remarks;
    std::string plugin, pluginopts;
    link = urlsafe_base64_decode(link.substr(6));
    jsondata.Parse(link.c_str());
    if(jsondata.HasParseError())
        return;
    if(!jsondata.HasMember("servers"))
        return;
    GetMember(jsondata, "airport", group);
    for(unsigned int i = 0; i < jsondata["servers"].Size(); i++)
    {
        //get default info
        GetMember(jsondata, "port", port);
        GetMember(jsondata, "encryption", method);
        GetMember(jsondata, "password", password);
        GetMember(jsondata, "plugin", plugin);
        GetMember(jsondata, "plugin_options", pluginopts);

        //get server-specific info
        jsondata["servers"][i]["server"] >> server;
        GetMember(jsondata["servers"][i], "remarks", remarks);
        GetMember(jsondata["servers"][i], "port", port);
        GetMember(jsondata["servers"][i], "encryption", method);
        GetMember(jsondata["servers"][i], "password", password);
        GetMember(jsondata["servers"][i], "plugin", plugin);
        GetMember(jsondata["servers"][i], "plugin_options", pluginopts);

        if(custom_port.size())
            port = custom_port;

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.group = group;
        node.remarks = remarks;
        node.server = server;
        node.port = to_int(port);
        node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
        node.id = index;
        nodes.push_back(node);
        index++;
    }
    return;
}

void explodeSSAndroid(std::string ss, bool libev, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes)
{
    Document json;
    nodeInfo node;
    int index = nodes.size();
    //first add some extra data before parsing
    ss = "{\"nodes\":" + ss + "}";
    json.Parse(ss.data());
    if(json.HasParseError())
        return;

    for(unsigned int i = 0; i < json["nodes"].Size(); i++)
    {
        std::string ps, password, method, server, port, group = SS_DEFAULT_GROUP;
        std::string plugin, pluginopts;

        server = GetMember(json["nodes"][i], "server");
        if(server.empty())
            continue;
        ps = GetMember(json["nodes"][i], "remarks");
        if(custom_port.size())
            port = custom_port;
        else
            port = GetMember(json["nodes"][i], "server_port");
        if(ps.empty())
            ps = server + ":" + port;
        password = GetMember(json["nodes"][i], "password");
        method = GetMember(json["nodes"][i], "method");
        plugin = GetMember(json["nodes"][i], "plugin");
        pluginopts = GetMember(json["nodes"][i], "plugin_opts");

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.id = index;
        node.group = group;
        node.remarks = ps;
        node.server = server;
        node.port = to_int(port);
        node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, ps, local_port, libev);
        nodes.push_back(node);
        index++;
    }
}

void explodeSSConf(std::string content, std::string custom_port, int local_port, bool libev, std::vector<nodeInfo> &nodes)
{
    nodeInfo node;
    Document json;
    std::string ps, password, method, server, port, plugin, pluginopts, group = SS_DEFAULT_GROUP;
    int index = nodes.size();

    json.Parse(content.data());
    if(json.HasParseError())
        return;
    if(json.HasMember("local_port") && json.HasMember("local_address")) //single libev config
    {
        server = GetMember(json, "server");
        port = GetMember(json, "server_port");
        json["local_port"].SetInt(local_port);
        json["local_address"].SetString("127.0.0.1");
        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.group = SS_DEFAULT_GROUP;
        node.remarks = server + ":" + port;
        node.server = server;
        node.port = to_int(port);
        node.proxyStr = SerializeObject(json);
        nodes.push_back(node);
        return;
    }
    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        json["configs"][i]["remarks"] >> ps;
        if(custom_port.size())
            port = custom_port;
        else
            json["configs"][i]["server_port"] >> port;
        if(ps.empty())
        {
            ps = server + ":" + port;
        }

        json["configs"][i]["password"] >> password;
        json["configs"][i]["method"] >> method;
        json["configs"][i]["server"] >> server;
        json["configs"][i]["plugin"] >> plugin;
        json["configs"][i]["plugin_opts"] >> pluginopts;

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.group = group;
        node.remarks = ps;
        node.id = index;
        node.server = server;
        node.port = to_int(port);
        node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, ps, local_port, libev);
        nodes.push_back(node);
        index++;
    }
    return;
}

void explodeSSR(std::string ssr, bool ss_libev, bool ssr_libev, std::string custom_port, int local_port, nodeInfo &node)
{
    std::string strobfs;
    std::vector<std::string> strcfg;
    std::string remarks, group, server, port, method, password, protocol, protoparam, obfs, obfsparam, remarks_base64;
    ssr = replace_all_distinct(ssr.substr(6), "\r", "");
    ssr = urlsafe_base64_decode(ssr);
    if(strFind(ssr, "/?"))
    {
        strobfs = ssr.substr(ssr.find("/?") + 2);
        ssr = ssr.substr(0, ssr.find("/?"));
        group = urlsafe_base64_decode(getUrlArg(strobfs, "group"));
        remarks = urlsafe_base64_decode(getUrlArg(strobfs, "remarks"));
        remarks_base64 = urlsafe_base64_reverse(getUrlArg(strobfs, "remarks"));
        obfsparam = regReplace(urlsafe_base64_decode(getUrlArg(strobfs, "obfsparam")), "\\s", "");
        protoparam = regReplace(urlsafe_base64_decode(getUrlArg(strobfs, "protoparam")), "\\s", "");
    }

    ssr = regReplace(ssr, "(.*):(.*?):(.*?):(.*?):(.*?):(.*)", "$1|$2|$3|$4|$5|$6");
    strcfg = split(ssr, "|");

    if(strcfg.size() != 6)
        return;

    server = strcfg[0];
    port = custom_port.empty() ? strcfg[1] : custom_port;
    protocol = strcfg[2];
    method = strcfg[3];
    obfs = strcfg[4];
    password = urlsafe_base64_decode(strcfg[5]);

    if(group.empty())
        group = SSR_DEFAULT_GROUP;
    if(remarks.empty())
    {
        remarks = server + ":" + port;
        remarks_base64 = base64_encode(remarks);
    }

    node.group = group;
    node.remarks = remarks;
    node.server = server;
    node.port = to_int(port, 0);
    if(find(ss_ciphers.begin(), ss_ciphers.end(), method) != ss_ciphers.end() && (obfs.empty() || obfs == "plain") && (protocol.empty() || protocol == "origin"))
    {
        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.proxyStr = ssConstruct(server, port, password, method, "", "", remarks, local_port, ss_libev);
    }
    else
    {
        node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
        node.proxyStr = ssrConstruct(group, remarks, remarks_base64, server, port, protocol, method, obfs, password, obfsparam, protoparam, local_port, ssr_libev);
    }
}

void explodeSSRConf(std::string content, std::string custom_port, int local_port, bool ss_libev, bool ssr_libev, std::vector<nodeInfo> &nodes)
{
    nodeInfo node;
    Document json;
    std::string remarks, remarks_base64, group, server, port, method, password, protocol, protoparam, obfs, obfsparam;
    int index = nodes.size();

    json.Parse(content.data());
    if(json.HasParseError())
        return;
    if(json.HasMember("local_port") && json.HasMember("local_address")) //single libev config
    {
        method = GetMember(json, "method");
        obfs = GetMember(json, "obfs");
        protocol = GetMember(json, "protocol");
        if(find(ss_ciphers.begin(), ss_ciphers.end(), method) != ss_ciphers.end() && (obfs.empty() || obfs == "plain") && (protocol.empty() || protocol == "origin"))
        {
            explodeSSConf(content, custom_port, local_port, ss_libev, nodes);
            return;
        }
        server = GetMember(json, "server");
        port = GetMember(json, "server_port");
        json["local_port"].SetInt(local_port);
        json["local_address"].SetString("127.0.0.1");
        node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
        node.group = SSR_DEFAULT_GROUP;
        node.remarks = server + ":" + port;
        node.server = server;
        node.port = to_int(port);
        node.proxyStr = SerializeObject(json);
        nodes.push_back(node);
        return;
    }
    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        json["configs"][i]["group"] >> group;
        if(group.empty())
            group = SSR_DEFAULT_GROUP;
        json["configs"][i]["remarks"] >> remarks;
        json["configs"][i]["server"] >> server;
        if(custom_port.size())
            port = custom_port;
        else
            json["configs"][i]["server_port"] >> port;
        if(remarks.empty())
            remarks = server + ":" + port;

        json["configs"][i]["remarks_base64"] >> remarks_base64;
        json["configs"][i]["password"] >> password;
        json["configs"][i]["method"] >> method;

        json["configs"][i]["protocol"] >> protocol;
        json["configs"][i]["protocolparam"] >> protoparam;
        json["configs"][i]["obfs"] >> obfs;
        json["configs"][i]["obfsparam"] >> obfsparam;

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
        node.group = group;
        node.remarks = remarks;
        node.id = index;
        node.server = server;
        node.port = to_int(port);
        node.proxyStr = ssrConstruct(group, remarks, remarks_base64, server, port, protocol, method, obfs, password, obfsparam, protoparam, local_port, ssr_libev);
        nodes.push_back(node);
        index++;
    }
    return;
}

void explodeSocks(std::string link, std::string custom_port, nodeInfo &node)
{
    std::string remarks, server, port, username, password;
    if(strFind(link, "socks://")) //v2rayn socks link
    {
        std::vector<std::string> arguments;
        if(strFind(link, "#"))
        {
            remarks = UrlDecode(link.substr(link.find("#") + 1));
            link = link.substr(0, link.find("#"));
        }
        link = urlsafe_base64_decode(link.substr(8));
        arguments = split(link, ":");
        if(arguments.size() < 2)
            return;
        server = arguments[0];
        port = arguments[1];
    }
    else if(strFind(link, "https://t.me/socks") || strFind(link, "tg://socks")) //telegram style socks link
    {
        server = getUrlArg(link, "server");
        port = getUrlArg(link, "port");
        username = getUrlArg(link, "user");
        password = getUrlArg(link, "pass");
    }
    if(remarks.empty())
        remarks = server + ":" + port;
    if(custom_port.size())
        port = custom_port;

    node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
    node.group = SOCKS_DEFAULT_GROUP;
    node.remarks = remarks;
    node.server = server;
    node.port = to_int(port, 0);
    node.proxyStr = socksConstruct(remarks, server, port, username, password);
}

void explodeHTTP(std::string link, std::string custom_port, nodeInfo &node)
{
    std::string remarks, server, port, username, password;
    server = getUrlArg(link, "server");
    port = getUrlArg(link, "port");
    username = getUrlArg(link, "user");
    password = getUrlArg(link, "pass");
    remarks = UrlDecode(getUrlArg(link, "remark"));

    if(remarks.empty())
        remarks = server + ":" + port;
    if(custom_port.size())
        port = custom_port;

    node.linkType = SPEEDTEST_MESSAGE_FOUNDHTTP;
    node.group = HTTP_DEFAULT_GROUP;
    node.remarks = remarks;
    node.server = server;
    node.port = to_int(port, 0);
    node.proxyStr = httpConstruct(remarks, server, port, username, password, strFind(link, "/https"));
}

void explodeQuan(std::string quan, std::string custom_port, int local_port, nodeInfo &node)
{
    std::string strTemp, itemName, itemVal;
    std::string group = V2RAY_DEFAULT_GROUP, ps, add, port, cipher = "auto", type = "none", id, aid = "0", net = "tcp", path, host, edge, tls;
    std::vector<std::string> configs, vArray;
    strTemp = regReplace(quan, "(.*?) = (.*)", "$1,$2");
    configs = split(strTemp, ",");

    if(configs[1] == "vmess")
    {
        if(configs.size() < 6)
            return;
        ps = trim(configs[0]);
        add = trim(configs[2]);
        port = custom_port.size() ? custom_port : trim(configs[3]);
        cipher = trim(configs[4]);
        id = trim(replace_all_distinct(configs[5], "\"", ""));

        //read link
        for(unsigned int i = 6; i < configs.size(); i++)
        {
            vArray = split(configs[i], "=");
            if(vArray.size() < 2)
                continue;
            itemName = trim(vArray[0]);
            itemVal = trim(vArray[1]);
            if(itemName == "group")
                group = itemVal;
            else if(itemName == "over-tls")
                tls = itemVal == "true" ? "tls" : "";
            else if(itemName == "tls-host")
                host = itemVal;
            else if(itemName == "obfs-path")
                path = replace_all_distinct(itemVal, "\"", "");
            else if(itemName == "obfs-header")
            {
                std::vector<std::string> headers = split(replace_all_distinct(replace_all_distinct(itemVal, "\"", ""), "[Rr][Nn]", "|"), "|");
                for(unsigned int j = 0; j < headers.size(); j++)
                {
                    if(strFind(headers[j], "Host: "))
                        host = headers[j].substr(6);
                    else if(strFind(headers[j], "Edge: "))
                        edge = headers[j].substr(6);
                }
            }
            else if(itemName == "obfs" && itemVal == "ws")
                net = "ws";
        }
        if(path.empty())
            path = "/";

        node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
        node.group = group;
        node.remarks = ps;
        node.server = add;
        node.port = to_int(port, 0);
        node.proxyStr = vmessConstruct(add, port, type, id, aid, net, cipher, path, host, edge, tls, local_port);
    }
}

void explodeNetch(std::string netch, bool ss_libev, bool ssr_libev, std::string custom_port, int local_port, nodeInfo &node)
{
    Document json;
    std::string type, remark, address, port, username, password, method, plugin, pluginopts, protocol, protoparam, obfs, obfsparam, id, aid, transprot, faketype, host, edge, path, tls;
    netch = urlsafe_base64_decode(netch.substr(8));

    json.Parse(netch.data());
    if(json.HasParseError())
        return;
    json["Type"] >> type;
    json["Remark"] >> remark;
    json["Hostname"] >> address;
    port = custom_port.size() ? custom_port : GetMember(json, "Port");
    method = GetMember(json, "EncryptMethod");
    password = GetMember(json, "Password");
    if(remark.empty())
        remark = address + ":" + port;
    if(type == "SS")
    {
        plugin = GetMember(json, "Plugin");
        pluginopts = GetMember(json, "PluginOption");
        node.group = SS_DEFAULT_GROUP;
        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.proxyStr = ssConstruct(address, port, password, method, plugin, pluginopts, remark, local_port, ss_libev);
    }
    else if(type == "SSR")
    {
        protocol = GetMember(json, "Protocol");
        obfs = GetMember(json, "OBFS");
        if(find(ss_ciphers.begin(), ss_ciphers.end(), method) != ss_ciphers.end() && (obfs.empty() || obfs == "plain") && (protocol.empty() || protocol == "origin"))
        {
            plugin = GetMember(json, "Plugin");
            pluginopts = GetMember(json, "PluginOption");
            node.group = SS_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
            node.proxyStr = ssConstruct(address, port, password, method, plugin, pluginopts, remark, local_port, ss_libev);
        }
        else
        {
            protoparam = GetMember(json, "ProtocolParam");
            obfsparam = GetMember(json, "OBFSParam");
            node.group = SSR_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
            node.proxyStr = ssrConstruct(SSR_DEFAULT_GROUP, remark, base64_encode(remark), address, port, protocol, method, obfs, password, obfsparam, protoparam, local_port, ssr_libev);
        }
    }
    else if(type == "VMess")
    {
        id = GetMember(json, "UserID");
        aid = GetMember(json, "AlterID");
        transprot = GetMember(json, "TransferProtocol");
        faketype = GetMember(json, "FakeType");
        host = GetMember(json, "Host");
        path = GetMember(json, "Path");
        edge = GetMember(json, "Edge");
        tls = GetMember(json, "TLSSecure") == "true" ? "tls" : "";
        node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
        node.group = V2RAY_DEFAULT_GROUP;
        node.proxyStr = vmessConstruct(address, port, faketype, id, aid, transprot, method, path, host, edge, tls, local_port);
    }
    else if(type == "Socks5")
    {
        username = GetMember(json, "Username");
        node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
        node.group = SOCKS_DEFAULT_GROUP;
        node.proxyStr = socksConstruct(remark, address, port, username, password);
    }
    else
        return;

    node.remarks = remark;
    node.server = address;
    node.port = (unsigned short)to_int(port, 0);;
}

void explodeClash(Node yamlnode, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes, bool ss_libev, bool ssr_libev)
{
    nodeInfo node;
    Node singleproxy;
    unsigned int index = nodes.size();
    const std::string section = yamlnode["proxies"].IsDefined() ? "proxies" : "Proxy";
    for(unsigned int i = 0; i < yamlnode[section].size(); i++)
    {
        std::string proxytype, ps, server, port, cipher, group, password; //common
        std::string type = "none", id, aid = "0", net = "tcp", path, host, edge, tls; //vmess
        std::string plugin, pluginopts, pluginopts_mode, pluginopts_host, pluginopts_mux; //ss
        std::string protocol, protoparam, obfs, obfsparam; //ssr
        std::string user; //socks

        singleproxy = yamlnode[section][i];
        singleproxy["type"] >> proxytype;
        singleproxy["name"] >> ps;
        singleproxy["server"] >> server;
        port = custom_port.empty() ? safe_as<std::string>(singleproxy["port"]) : custom_port;
        if(port.empty())
            continue;
        if(proxytype == "vmess")
        {
            group = V2RAY_DEFAULT_GROUP;

            singleproxy["uuid"] >> id;
            singleproxy["alterId"] >> aid;
            singleproxy["cipher"] >> cipher;
            net = singleproxy["network"].IsDefined() ? safe_as<std::string>(singleproxy["network"]) : "tcp";
            path = singleproxy["ws-path"].IsDefined() ? safe_as<std::string>(singleproxy["ws-path"]) : "/";
            if(singleproxy["tls"].IsDefined())
                tls = safe_as<std::string>(singleproxy["tls"]) == "true" ? "tls" : "";
            else
                tls.clear();
            if(singleproxy["ws-headers"].IsDefined())
            {
                singleproxy["ws-headers"]["Host"] >> host;
                singleproxy["ws-headers"]["Edge"] >> edge;
            }
            else
                host.clear();


            node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
            node.proxyStr = vmessConstruct(server, port, type, id, aid, net, cipher, path, host, edge, tls, local_port);
        }
        else if(proxytype == "ss")
        {
            group = SS_DEFAULT_GROUP;

            singleproxy["cipher"] >> cipher;
            singleproxy["password"] >> password;
            if(singleproxy["plugin"].IsDefined())
            {
                if(safe_as<std::string>(singleproxy["plugin"]) == "obfs")
                {
                    plugin = "simple-obfs";
                    if(singleproxy["plugin-opts"].IsDefined())
                    {
                        singleproxy["plugin-opts"]["mode"] >> pluginopts_mode;
                        if(singleproxy["plugin-opts"]["host"].IsDefined())
                            singleproxy["plugin-opts"]["host"] >> pluginopts_host;
                        else
                            pluginopts_host.clear();
                    }
                }
                else if(safe_as<std::string>(singleproxy["plugin"]) == "v2ray-plugin")
                {
                    plugin = "v2ray-plugin";
                    if(singleproxy["plugin-opts"].IsDefined())
                    {
                        singleproxy["plugin-opts"]["mode"] >> pluginopts_mode;
                        if(singleproxy["plugin-opts"]["host"].IsDefined())
                            singleproxy["plugin-opts"]["host"] >> pluginopts_host;
                        else
                            pluginopts_host.clear();
                        if(singleproxy["plugin-opts"]["tls"].IsDefined())
                            tls = safe_as<bool>(singleproxy["plugin-opts"]["tls"]) ? "tls;" : "";
                        else
                            tls.clear();
                        if(singleproxy["plugin-opts"]["path"].IsDefined())
                            singleproxy["plugin-opts"]["path"] >> path;
                        else
                            path.clear();
                        if(singleproxy["plugin-opts"]["mux"].IsDefined())
                            pluginopts_mux = safe_as<bool>(singleproxy["plugin-opts"]["mux"]) ? "mux=4;" : "";
                        else
                            pluginopts_mux.clear();
                    }
                }
            }
            else if(singleproxy["obfs"].IsDefined())
            {
                plugin = "simple-obfs";
                singleproxy["obfs"] >> pluginopts_mode;
                if(singleproxy["obfs-host"].IsDefined())
                    singleproxy["obfs-host"] >> pluginopts_host;
            }
            else
                plugin.clear();

            if(plugin == "simple-obfs")
            {
                pluginopts = "obfs=" + pluginopts_mode;
                pluginopts += pluginopts_host.empty() ? "" : ";obfs-host=" + pluginopts_host;
            }
            else if(plugin == "v2ray-plugin")
            {
                pluginopts = "mode=" + pluginopts_mode + ";" + tls + pluginopts_mux;
                if(pluginopts_host.size())
                    pluginopts += "host=" + pluginopts_host + ";";
                if(path.size())
                    pluginopts += "path=" + path + ";";
                if(pluginopts_mux.size())
                    pluginopts += "mux=" + pluginopts_mux + ";";
            }
            else
                pluginopts.clear();

            //support for go-shadowsocks2
            if(cipher == "AEAD_CHACHA20_POLY1305")
                cipher = "chacha20-ietf-poly1305";
            else if(strFind(cipher, "AEAD"))
            {
                cipher = replace_all_distinct(replace_all_distinct(cipher, "AEAD_", ""), "_", "-");
                std::transform(cipher.begin(), cipher.end(), cipher.begin(), ::tolower);
            }

            node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
            node.proxyStr = ssConstruct(server, port, password, cipher, plugin, pluginopts, ps, local_port, ss_libev);
        }
        else if(proxytype == "socks")
        {
            group = SOCKS_DEFAULT_GROUP;

            if(singleproxy["username"].IsDefined() && singleproxy["password"].IsDefined())
            {
                singleproxy["username"] >> user;
                singleproxy["password"] >> password;
            }
            else
            {
                user.clear();
                password.clear();
            }

            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.proxyStr = socksConstruct(ps, server, port, user, password);
        }
        else if(proxytype == "ssr")
        {
            group = SSR_DEFAULT_GROUP;

            singleproxy["cipher"] >> cipher;
            singleproxy["password"] >> password;
            singleproxy["protocol"] >> protocol;
            singleproxy["protocolparam"] >> protoparam;
            singleproxy["obfs"] >> obfs;
            singleproxy["obfsparam"] >> obfsparam;

            node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
            node.proxyStr = ssrConstruct(group, ps, base64_encode(ps), server, port, protocol, cipher, obfs, password, obfsparam, protoparam, local_port, ssr_libev);
        }
        else if(proxytype == "http")
        {
            group = HTTP_DEFAULT_GROUP;

            if(singleproxy["username"].IsDefined() && singleproxy["password"].IsDefined())
            {
                singleproxy["username"] >> user;
                singleproxy["password"] >> password;
            }
            else
            {
                user.clear();
                password.clear();
            }

            node.linkType = SPEEDTEST_MESSAGE_FOUNDHTTP;
            node.proxyStr = httpConstruct(ps, server, port, user, password);
        }
        else
            continue;

        node.group = group;
        node.remarks = ps;
        node.server = server;
        node.port = to_int(port);
        node.id = index;
        nodes.push_back(node);
        index++;
    }
    return;
}

void explodeShadowrocket(std::string rocket, std::string custom_port, int local_port, nodeInfo &node)
{
    std::string add, port, type, id, aid, net = "tcp", path, host, tls, cipher, remarks;
    std::string obfs; //for other style of link
    std::string addition;
    string_array userinfo;
    rocket = rocket.substr(8);

    addition = rocket.substr(rocket.find("?") + 1);
    rocket = rocket.substr(0, rocket.find("?"));

    userinfo = split(regReplace(urlsafe_base64_decode(rocket), "(.*?):(.*?)@(.*):(.*)", "$1,$2,$3,$4"), ",");
    if(userinfo.size() != 4) // broken link
        return;
    cipher = userinfo[0];
    id = userinfo[1];
    add = userinfo[2];
    port = custom_port.size() ? custom_port : userinfo[3];
    remarks = UrlDecode(getUrlArg(addition, "remark"));
    obfs = getUrlArg(addition, "obfs");
    if(obfs.size())
    {
        if(obfs == "websocket")
        {
            net = "ws";
            host = getUrlArg(addition, "obfsParam");
            path = getUrlArg(addition, "path");
        }
    }
    else
    {
        net = getUrlArg(addition, "network");
        host = getUrlArg(addition, "wsHost");
        path = getUrlArg(addition, "wspath");
    }
    tls = getUrlArg(addition, "tls") == "1" ? "tls" : "";
    aid = getUrlArg(addition, "aid");

    if(aid.empty())
        aid = "0";

    if(remarks.empty())
        remarks = add + ":" + port;

    node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
    node.group = V2RAY_DEFAULT_GROUP;
    node.remarks = remarks;
    node.server = add;
    node.port = to_int(port, 0);
    node.proxyStr = vmessConstruct(add, port, type, id, aid, net, cipher, path, host, "", tls, local_port);
}

void explodeKitsunebi(std::string kit, std::string custom_port, int local_port, nodeInfo &node)
{
    std::string add, port, type, id, aid = "0", net = "tcp", path, host, tls, cipher = "auto", remarks;
    std::string addition;
    string_array userinfo;
    kit = kit.substr(9);

    if(strFind(kit, "#"))
    {
        remarks = kit.substr(kit.find("#") + 1);
        kit = kit.substr(0, kit.find("#"));
    }

    addition = kit.substr(kit.find("?") + 1);
    kit = kit.substr(0, kit.find("?"));

    userinfo = split(regReplace(kit, "(.*?)@(.*):(.*)", "$1,$2,$3"), ",");
    if(userinfo.size() != 3)
        return;
    id = userinfo[0];
    add = userinfo[1];
    if(strFind(userinfo[2], "/"))
    {
        port = userinfo[2].substr(0, userinfo[2].find("/"));
        path = userinfo[2].substr(userinfo[2].find("/"));
    }
    else
    {
        port = userinfo[2];
    }
    if(custom_port.size())
        port = custom_port;
    net = getUrlArg(addition, "network");
    tls = getUrlArg(addition, "tls") == "true" ? "tls" : "";
    host = getUrlArg(addition, "ws.host");

    if(remarks.empty())
        remarks = add + ":" + port;

    node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
    node.group = V2RAY_DEFAULT_GROUP;
    node.remarks = remarks;
    node.server = add;
    node.port = to_int(port, 0);
    node.proxyStr = vmessConstruct(add, port, type, id, aid, net, cipher, path, host, "", tls, local_port);
}

bool explodeSurge(std::string surge, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes, bool libev)
{
    std::multimap<std::string, std::string> proxies;
    nodeInfo node;
    unsigned int i, index = nodes.size();
    INIReader ini;

    /*
    if(!strFind(surge, "[Proxy]"))
        return false;
    */

    ini.store_isolated_line = true;
    ini.keep_empty_section = false;
    ini.SetIsolatedItemsSection("Proxy");
    ini.IncludeSection("Proxy");
    ini.AddDirectSaveSection("Proxy");
    ini.Parse(surge);

    if(!ini.SectionExist("Proxy"))
        return false;
    ini.EnterSection("Proxy");
    ini.GetItems(proxies);

    const std::string proxystr = "(.*?)\\s*=\\s*(.*)";

    for(auto &x : proxies)
    {
        std::string remarks, server, port, method, username, password; //common
        std::string plugin, pluginopts, pluginopts_mode, pluginopts_host = "cloudfront.net", mod_url, mod_md5; //ss
        std::string id, net, tls, host, edge, path; //v2
        std::string protocol, protoparam; //ssr
        std::string itemName, itemVal;
        std::vector<std::string> configs, vArray, headers, header;

        remarks = regReplace(x.second, proxystr, "$1");
        configs = split(regReplace(x.second, proxystr, "$2"), ",");
        if(configs.size() < 2 || configs[0] == "direct")
            continue;
        if(configs[0] == "custom") //surge 2 style custom proxy
        {
            //remove module detection to speed up parsing and compatible with broken module
            /*
            mod_url = trim(configs[5]);
            if(parsedMD5.count(mod_url) > 0)
            {
                mod_md5 = parsedMD5[mod_url]; //read calculated MD5 from map
            }
            else
            {
                mod_md5 = getMD5(webGet(mod_url)); //retrieve module and calculate MD5
                parsedMD5.insert(std::pair<std::string, std::string>(mod_url, mod_md5)); //save unrecognized module MD5 to map
            }
            */

            //if(mod_md5 == modSSMD5) //is SSEncrypt module
            {
                server = trim(configs[1]);
                port = custom_port.empty() ? trim(configs[2]) : custom_port;
                method = trim(configs[3]);
                password = trim(configs[4]);

                for(i = 6; i < configs.size(); i++)
                {
                    vArray = split(configs[i], "=");
                    if(vArray.size() < 2)
                        continue;
                    itemName = trim(vArray[0]);
                    itemVal = trim(vArray[1]);
                    if(itemName == "obfs")
                    {
                        plugin = "simple-obfs";
                        pluginopts_mode = itemVal;
                    }
                    else if(itemName == "obfs-host")
                        pluginopts_host = itemVal;
                }
                if(plugin.size())
                {
                    pluginopts = "obfs=" + pluginopts_mode;
                    pluginopts += pluginopts_host.empty() ? "" : ";obfs-host=" + pluginopts_host;
                }

                node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
                node.group = SS_DEFAULT_GROUP;
                node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
            }
            //else
            //    continue;
        }
        else if(configs[0] == "ss") //surge 3 style ss proxy
        {
            server = trim(configs[1]);
            port = custom_port.empty() ? trim(configs[2]) : custom_port;

            for(i = 3; i < configs.size(); i++)
            {
                vArray = split(configs[i], "=");
                if(vArray.size() < 2)
                    continue;
                itemName = trim(vArray[0]);
                itemVal = trim(vArray[1]);
                if(itemName == "encrypt-method")
                    method = itemVal;
                else if(itemName == "password")
                    password = itemVal;
                else if(itemName == "obfs")
                {
                    plugin = "simple-obfs";
                    pluginopts_mode = itemVal;
                }
                else if(itemName == "obfs-host")
                    pluginopts_host = itemVal;
            }
            if(plugin.size())
            {
                pluginopts = "obfs=" + pluginopts_mode;
                pluginopts += pluginopts_host.empty() ? "" : ";obfs-host=" + pluginopts_host;
            }

            node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
            node.group = SS_DEFAULT_GROUP;
            node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
        }
        else if(configs[0] == "socks5") //surge 3 style socks5 proxy
        {
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.group = SOCKS_DEFAULT_GROUP;
            server = trim(configs[1]);
            port = custom_port.empty() ? trim(configs[2]) : custom_port;
            if(configs.size() >= 5)
            {
                username = trim(configs[2]);
                password = trim(configs[3]);
            }
            node.proxyStr = socksConstruct(remarks, server, port, username, password);
        }
        else if(configs[0] == "vmess") //surge 4 style vmess proxy
        {
            server = trim(configs[1]);
            port = custom_port.empty() ? trim(configs[2]) : custom_port;
            net = "tcp";
            method = "auto";

            for(i = 3; i < configs.size(); i++)
            {
                vArray = split(configs[i], "=");
                if(vArray.size() != 2)
                    continue;
                itemName = trim(vArray[0]);
                itemVal = trim(vArray[1]);
                if(itemName == "username")
                    id = itemVal;
                else if(itemName == "ws")
                {
                    net = itemVal == "true" ? "ws" : "tcp";
                }
                else if(itemName == "tls")
                {
                    tls = itemVal == "true" ? "tls" : "";
                }
                else if(itemName == "ws-path")
                    path = itemVal;
                else if(itemName == "ws-headers")
                {
                    headers = split(itemVal, "|");
                    for(auto &y : headers)
                    {
                        header = split(trim(y), ":");
                        if(regMatch(header[0], "(?i)host"))
                            host = header[1];
                        else if(regMatch(header[0], "(?i)edge"))
                            edge = header[1];
                    }
                }
            }
            if(host.empty() && !isIPv4(server) && !isIPv6(server))
                host = server;

            node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
            node.group = V2RAY_DEFAULT_GROUP;
            node.proxyStr = vmessConstruct(server, port, "", id, "0", net, method, path, host, edge, tls, local_port);
        }
        else if(configs[0] == "http") //http proxy
        {
            node.linkType = SPEEDTEST_MESSAGE_FOUNDHTTP;
            node.group = HTTP_DEFAULT_GROUP;
            server = trim(configs[1]);
            port = custom_port.empty() ? trim(configs[2]) : custom_port;
            if(configs.size() >= 5)
            {
                username = trim(configs[2]);
                password = trim(configs[3]);
            }
            node.proxyStr = httpConstruct(remarks, server, port, username, password);
        }
        else if(remarks == "shadowsocks") //quantumult x style ss/ssr link
        {
            server = trim(configs[0].substr(0, configs[0].rfind(":")));
            port = custom_port.empty() ? trim(configs[0].substr(configs[0].rfind(":") + 1)) : custom_port;

            for(i = 1; i < configs.size(); i++)
            {
                vArray = split(trim(configs[i]), "=");
                if(vArray.size() != 2)
                    continue;
                itemName = trim(vArray[0]);
                itemVal = trim(vArray[1]);
                if(itemName == "method")
                    method = itemVal;
                else if(itemName == "password")
                    password = itemVal;
                else if(itemName == "tag")
                    remarks = itemVal;
                else if(itemName == "ssr-protocol")
                    protocol = itemVal;
                else if(itemName == "ssr-protocol-param")
                    protoparam = itemVal;
                else if(itemName == "obfs")
                {
                    plugin = "simple-obfs";
                    pluginopts_mode = itemVal;
                }
                else if(itemName == "obfs-host")
                    pluginopts_host = itemVal;
            }
            if(remarks.empty())
                remarks = server + ":" + port;
            if(plugin.size())
            {
                pluginopts = "obfs=" + pluginopts_mode;
                if(pluginopts_host.size())
                    pluginopts += ";obfs-host=" + pluginopts_host;
            }

            if(protocol.size())
            {
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
                node.group = SSR_DEFAULT_GROUP;
                node.proxyStr = ssrConstruct(node.group, remarks, base64_encode(remarks), server, port, protocol, method, pluginopts_mode, password, pluginopts_host, protoparam, local_port, libev);
            }
            else
            {
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
                node.group = SS_DEFAULT_GROUP;
                node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
            }
        }
        else if(remarks == "vmess") //quantumult x style vmess link
        {
            server = trim(configs[0].substr(0, configs[0].rfind(":")));
            port = custom_port.empty() ? trim(configs[0].substr(configs[0].rfind(":") + 1)) : custom_port;
            net = "tcp";

            for(i = 1; i < configs.size(); i++)
            {
                vArray = split(trim(configs[i]), "=");
                if(vArray.size() != 2)
                    continue;
                itemName = trim(vArray[0]);
                itemVal = trim(vArray[1]);
                if(itemName == "method")
                    method = itemVal;
                else if(itemName == "password")
                    id = itemVal;
                else if(itemName == "tag")
                    remarks = itemVal;
                else if(itemName == "obfs")
                {
                    if(itemVal == "ws")
                        net = "ws";
                    else if(itemVal == "over-tls")
                        tls = "tls";
                }
                else if(itemName == "obfs-host")
                    host = itemVal;
                else if(itemName == "obfs-uri")
                    path = itemVal;
                else if(itemName == "over-tls")
                    tls = itemVal == "true" ? "tls" : "";
            }
            if(remarks.empty())
                remarks = server + ":" + port;

            if(host.empty() && !isIPv4(server) && !isIPv6(server))
                host = server;

            node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
            node.group = V2RAY_DEFAULT_GROUP;
            node.proxyStr = vmessConstruct(server, port, "", id, "0", net, method, path, host, "", tls, local_port);
        }
        else
            continue;

        node.remarks = remarks;
        node.server = server;
        node.port = to_int(port);
        node.id = index;
        nodes.push_back(node);
        index++;
    }
    return index;
}

void explodeSSTap(std::string sstap, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes, bool ss_libev, bool ssr_libev)
{
    Document json;
    nodeInfo node;
    unsigned int index = nodes.size();
    json.Parse(sstap.data());
    if(json.HasParseError())
        return;

    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        std::string configType, group, remarks, server, port;
        std::string cipher;
        std::string user, pass;
        std::string protocol, protoparam, obfs, obfsparam;

        json["configs"][i]["group"] >> group;
        json["configs"][i]["remarks"] >> remarks;
        json["configs"][i]["server"] >> server;
        port = custom_port.size() ? custom_port : GetMember(json["configs"][i], "server_port");

        if(remarks.empty())
            remarks = server + ":" + port;

        json["configs"][i]["password"] >> pass;
        json["configs"][i]["type"] >> configType;
        if(configType == "5") //socks 5
        {
            json["configs"][i]["username"] >> user;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.proxyStr = socksConstruct(remarks, server, port, user, pass);
        }
        else if(configType == "6") //ss/ssr
        {
            json["configs"][i]["protocol"] >> protocol;
            json["configs"][i]["obfs"] >> obfs;
            json["configs"][i]["method"] >> cipher;
            if(find(ss_ciphers.begin(), ss_ciphers.end(), cipher) != ss_ciphers.end() && protocol == "origin" && obfs == "plain") //is ss
            {
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
                node.proxyStr = ssConstruct(server, port, pass, cipher, "", "", remarks, local_port, ss_libev);
            }
            else //is ssr cipher
            {
                json["configs"][i]["obfsparam"] >> obfsparam;
                json["configs"][i]["protocolparam"] >> protoparam;
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
                node.proxyStr = ssrConstruct(group, remarks, base64_encode(remarks), server, port, protocol, cipher, obfs, pass, obfsparam, protoparam, local_port, ssr_libev);
            }
        }
        else
            continue;

        node.group = group;
        node.remarks = remarks;
        node.id = index;
        node.server = server;
        node.port = to_int(port);
        nodes.push_back(node);
    }
}

void explodeNetchConf(std::string netch, bool ss_libev, bool ssr_libev, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes)
{
    Document json;
    nodeInfo node;
    unsigned int index = nodes.size();

    json.Parse(netch.data());
    if(json.HasParseError())
        return;

    if(!json.HasMember("Server"))
        return;

    for(unsigned int i = 0; i < json["Server"].Size(); i++)
    {
        explodeNetch("Netch://" + base64_encode(SerializeObject(json["Server"][i])), ss_libev, ssr_libev, custom_port, local_port, node);

        node.id = index;
        nodes.push_back(node);
        index++;
    }
}

bool chkIgnore(const nodeInfo &node, string_array &exclude_remarks, string_array &include_remarks)
{
    bool excluded = false, included = false;
    //std::string remarks = UTF8ToGBK(node.remarks);
    std::string remarks = node.remarks;
    writeLog(LOG_TYPE_INFO, "Comparing exclude remarks...");
    excluded = std::any_of(exclude_remarks.cbegin(), exclude_remarks.cend(), [&remarks](auto &x)
    {
        return regFind(remarks, x);
    });
    if(include_remarks.size() != 0)
    {
        writeLog(LOG_TYPE_INFO, "Comparing include remarks...");
        included = std::any_of(include_remarks.cbegin(), include_remarks.cend(), [&remarks](auto &x)
        {
            return regFind(remarks, x);
        });
    }
    else
    {
        included = true;
    }

    return excluded || !included;
}

int explodeConf(std::string filepath, std::string custom_port, int local_port, bool sslibev, bool ssrlibev, std::vector<nodeInfo> &nodes, string_array &exclude_remarks, string_array &include_remarks)
{
    std::ifstream infile;
    std::stringstream contentstrm;
    infile.open(filepath);

    contentstrm << infile.rdbuf();
    infile.close();

    return explodeConfContent(contentstrm.str(), custom_port, local_port, sslibev, ssrlibev, nodes, exclude_remarks, include_remarks);
}

int explodeConfContent(std::string content, std::string custom_port, int local_port, bool sslibev, bool ssrlibev, std::vector<nodeInfo> &nodes, string_array &exclude_remarks, string_array &include_remarks)
{
    unsigned int index = 0;
    int filetype = -1;
    std::vector<nodeInfo>::iterator iter;

    if(strFind(content, "\"version\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSS;
    else if(strFind(content, "\"serverSubscribes\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSR;
    else if(strFind(content, "\"uiItem\"") || strFind(content, "vnext"))
        filetype = SPEEDTEST_MESSAGE_FOUNDVMESS;
    else if(strFind(content, "\"proxy_apps\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSCONF;
    else if(strFind(content, "\"idInUse\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSTAP;
    else if(strFind(content, "\"local_address\"") && strFind(content, "\"local_port\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSR; //use ssr config parser
    else if(strFind(content, "\"ModeFileNameType\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDNETCH;

    switch(filetype)
    {
    case SPEEDTEST_MESSAGE_FOUNDSS:
        explodeSSConf(content, custom_port, local_port, sslibev, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        explodeSSRConf(content, custom_port, local_port, sslibev, ssrlibev, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        explodeVmessConf(content, custom_port, local_port, sslibev, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSCONF:
        explodeSSAndroid(content, sslibev, custom_port, local_port, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSTAP:
        explodeSSTap(content, custom_port, local_port, nodes, sslibev, ssrlibev);
        break;
    case SPEEDTEST_MESSAGE_FOUNDNETCH:
        explodeNetchConf(content, sslibev, ssrlibev, custom_port, local_port, nodes);
        break;
    default:
        //try to parse as a local subscription
        explodeSub(content, sslibev, ssrlibev, custom_port, local_port, nodes, exclude_remarks, include_remarks);
        if(nodes.size() == 0)
            return SPEEDTEST_ERROR_UNRECOGFILE;
        else
            return SPEEDTEST_ERROR_NONE;
    }

    iter = nodes.begin();
    while(iter != nodes.end())
    {
        if(chkIgnore(*iter, exclude_remarks, include_remarks))
        {
            writeLog(LOG_TYPE_INFO, "Node  " + iter->group + " - " + iter->remarks + "  has been ignored and will not be added.");
            nodes.erase(iter);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Node  " + iter->group + " - " + iter->remarks + "  has been added.");
            iter->id = index;
            ++index;
            ++iter;
        }
    }

    return SPEEDTEST_ERROR_NONE;
}

void explode(std::string link, bool sslibev, bool ssrlibev, std::string custom_port, int local_port, nodeInfo &node)
{
    if(strFind(link, "ssr://"))
        explodeSSR(link, sslibev, ssrlibev, custom_port, local_port, node);
    else if(strFind(link, "vmess://") || strFind(link, "vmess1://"))
        explodeVmess(link, custom_port, local_port, node);
    else if(strFind(link, "ss://"))
        explodeSS(link, sslibev, custom_port, local_port, node);
    else if(strFind(link, "socks://") || strFind(link, "https://t.me/socks") || strFind(link, "tg://socks"))
        explodeSocks(link, custom_port, node);
    else if(strFind(link, "Netch://"))
        explodeNetch(link, sslibev, ssrlibev, custom_port, local_port, node);
}

void explodeSub(std::string sub, bool sslibev, bool ssrlibev, std::string custom_port, int local_port, std::vector<nodeInfo> &nodes, string_array &exclude_remarks, string_array &include_remarks)
{
    std::stringstream strstream;
    std::string strLink;
    bool processed = false;
    nodeInfo node;

    //try to parse as SSD configuration
    if(strFind(sub, "ssd://"))
    {
        explodeSSD(sub, sslibev, custom_port, local_port, nodes);
        processed = true;
    }

    //try to parse as clash configuration
    try
    {
        if(!processed && strFind(sub, "Proxy"))
        {
            Node yamlnode = Load(sub);
            if(yamlnode.size() && yamlnode["Proxy"])
            {
                explodeClash(yamlnode, custom_port, local_port, nodes, sslibev, ssrlibev);
                processed = true;
            }
        }
    }
    catch (std::exception &e)
    {
        //ignore
    }

    //try to parse as surge configuration
    if(!processed && explodeSurge(sub, custom_port, local_port, nodes, sslibev))
    {
        processed = true;
    }

    if(!processed && explodeLog(sub, nodes) != -1)
    {
        processed = true;
    }

    //try to parse as normal subscription
    if(!processed)
    {
        sub = urlsafe_base64_decode(trim(sub));
        strstream << sub;
        char delimiter = count(sub.begin(), sub.end(), '\n') < 1 ? count(sub.begin(), sub.end(), '\r') < 1 ? ' ' : '\r' : '\n';
        while(getline(strstream, strLink, delimiter))
        {
            node.linkType = -1;
            explode(strLink, sslibev, ssrlibev, custom_port, local_port, node);
            if(strLink.size() == 0 || node.linkType == -1)
            {
                continue;
            }
            nodes.push_back(node);
        }
    }

    int index = 0;
    std::vector<nodeInfo>::iterator iter = nodes.begin();
    while(iter != nodes.end())
    {
        if(chkIgnore(*iter, exclude_remarks, include_remarks))
        {
            writeLog(LOG_TYPE_INFO, "Node  " + iter->group + " - " + iter->remarks + "  has been ignored and will not be added.");
            nodes.erase(iter);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Node  " + iter->group + " - " + iter->remarks + "  has been added.");
            iter->id = index;
            ++index;
            ++iter;
        }
    }
}
