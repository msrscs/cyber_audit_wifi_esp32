

# Cyber Audit WiFi ESP32

## Faculdade: Cesar School
## Curso: Segurança da Informação
## Período: 2025.2
## Disciplina: Algoritmos e Estrutura de Dados
## Professor: Ferando Ferreira De Carvalho
## Projeto: CyberAuditESP32
## Equipe:
##        -  Artur Torres Lima Cavalcanti
##        -  Carlos Vinicius Alves de Figueiredo
##        -  Eduardo Henrique Ferreira Fonseca Barbosa
##        -  Gabriel de Medeiros Almeida
##        -  Mauro Sérgio Rezende da Silva
##        -  Silvio Barros Tenório

## 📋 Descrição do Projeto

Este projeto implementa um sistema completo de auditoria de segurança WiFi usando ESP32, desenvolvido exclusivamente para fins educativos e didáticos. O sistema demonstra conceitos fundamentais de segurança em redes sem fio através de três módulos principais:

### Funcionalidades Principais

1. **📡 Scanner de Redes WiFi**
   - Detecta e analisa redes WiFi no ambiente
   - Coleta informações detalhadas (SSID, BSSID, RSSI, Canal, Segurança)
   - Executa scans periódicos automáticos
   - Suporta detecção de até 50 redes simultaneamente

2. **🌐 Portal Captivo**
   - Simula um portal captivo realístico
   - Interface moderna inspirada em portais universitários
   - Demonstra técnicas de phishing WiFi
   - Captura credenciais para análise educativa
   - Sistema de armazenamento local com sincronização automática

3. **🔍 Análise de Vulnerabilidades**
   - Identifica redes abertas e protocolos fracos
   - Gera relatórios de segurança detalhados
   - Fornece recomendações personalizadas
   - Detecta padrões suspeitos e potenciais ameaças

4. **☁️ Integração Google Sheets**
   - Sincronização automática de dados capturados
   - Armazenamento em nuvem para análise posterior
   - Sistema de backup e recuperação
   - Interface web para visualização de dados


## 🛠️ Lista de Materiais

### Hardware Necessário
- **1x ESP32 Development Board** (NodeMCU ou similar)
- **1x Cabo USB** (tipo A para micro USB)
- **1x Computador** com Windows/Linux/macOS
- **1x Smartphone/Tablet** (para testes do portal captivo)

![ESP32 D](docs/esp32d.png)

### Software Necessário
- **PlatformIO IDE**
- **Visual Studio Code**
- **Google Account** (para integração com Sheets)

## 🚀 Instruções para Replicar

### Passo 1: Preparação do Ambiente

1. **Instale o PlatformIO**
   ```bash
   # No Visual Studio Code, instale a extensão PlatformIO
   ```

2. **Clone o Repositório**
   ```bash
   git clone https://github.com/msrscs/cyber_audit_wifi_esp32.git
   ```

### Passo 2: Configuração do Hardware

1. **Conecte o ESP32**
   - Conecte o ESP32 ao computador via cabo USB
   - Verifique se os drivers estão instalados
   - Identifique a porta COM (Windows) ou /dev/ttyUSB (Linux)

2. **Verificação da Conexão**
   ```bash
   # Teste a comunicação
   pio device list
   ```

### Passo 3: Configuração do Software

1. **Configure o arquivo `platformio.ini`**
   ```ini
   [env:esp32dev_alt]
   platform = espressif32
   board = esp32dev
   framework = arduino
   lib_deps = 
   	bblanchon/ArduinoJson@^6.21.3
   	WebServer
	   DNSServer
	   tzapu/WiFiManager@^2.0.17
   monitor_speed = 115200
   upload_speed = 115200
   upload_port = /dev/ttyACM0  ; Ajuste para sua porta, caso necessário
   monitor_port = /dev/ttyACM0 ; Ajuste para sua porta, caso necessário
   ```

2. **Instale as Dependências**
   ```bash
   pio lib install "WiFi" "WebServer" "DNSServer" "ArduinoJson" "HTTPClient" "WiFiManager"
   ```

### Passo 4: Configuração do Google Sheets (Opcional)

1. **Crie um Google Apps Script**
   - Acesse [script.google.com](https://script.google.com)
   - Crie um novo projeto
   - Cole o código de integração (fornecido separadamente)
   - Configure as permissões

2. **Configure a URL no código**
   ```cpp
   const char* googleScriptURL = "SUA_URL_AQUI";
   ```

### Passo 5: Compilação e Upload

1. **Compile o Projeto**
   ```bash
   pio run
   ```

2. **Faça o Upload**
   ```bash
   pio run --target upload
   ```

3. **Abra o Monitor Serial**
   ```bash
   pio device monitor --baud 115200
   ```

### Passo 6: Teste das Funcionalidades

1. **Teste o Scanner**
   - Digite `1` no monitor serial
   - Aguarde o scan automático das redes
   - Verifique a lista de redes detectadas

2. **Teste o Portal Captivo**
   - Digite `2` no monitor serial
   - Conecte um dispositivo na rede "Cesar_School_contingencia"
   - Tente acessar qualquer site
   - Observe o redirecionamento para o portal

3. **Teste a Análise de Segurança**
   - Digite `4` no monitor serial
   - Revise o relatório de vulnerabilidades
   - Implemente as recomendações sugeridas

### Passo 7: Monitoramento

1. **Interface Serial**
   ```
   Comandos disponíveis:
   1 - Modo Scanner de Redes
   2 - Portal Captivo Demo
   3 - Mostrar redes encontradas
   4 - Análise de segurança
   5 - Mostrar credenciais capturadas
   6 - Configurar WiFi ESP32
   7 - Conectar WiFi e sincronizar dados
   h - Mostrar este menu
   ```

## 📱 Printscreen do Projeto Funcionando

### 1. Início do Sistema
![Início do Sistema](docs/iniciodosistema.png)
*Inicialização do sistema conectando ao WiFi pré-configurado ou aguarda configurar uma nova rede no WiFi*
![Menu](docs/menu.png)
*Mostra o menu de comandos disponíveis*

### 2. Scanner de Redes WiFi
![Redes Encontradas](docs/redesencontradas.png)
*Resultado do scanner mostrando as redes WiFi detectadas*

![Redes Armazenadas](docs/redesencontradasarmazenadas.png)
![Redes Armazenadas 1](docs/redesencontradasarmazenadas1.png)
![Redes Armazenadas 2](docs/redesencontradasarmazenadas2.png)
![Redes Armazenadas 3](docs/redesencontradasarmazenadas3.png)
*Redes WiFi encontradas e armazenadas na memória do sistema*

### 3. Portal Captivo
![Iniciando Portal Captivo](docs/iniciandoportalcaptivo.png)
*Inicialização do módulo de Portal Captivo*

![ESP32 L](docs/esp32l.png)
*Led azul ligado no ESP32 sinalizando que está no modo Portal Captivo*

![Portal Captivo](docs/portalcaptivo.png)
*Interface do Portal Captivo apresentada aos usuários*
![Portal Captivo 1](docs/portalcaptivo1.png)

### 4. Armazenamento de Credenciais
![Credenciais Capturadas](docs/credenciaiscapturadas.png)
![Credenciais Capturadas 1](docs/credenciaiscapturadas1.png)
*Visualização das credenciais capturadas através do Portal Captivo*

![Persistência de Credenciais](docs/persistenciadecredenciais.png)
![Persistência de Credenciais 1](docs/persistenciadecredenciaisN.png)
*Sistema de persistência que mantém as credenciais mesmo após reinicialização*

![Credenciais Armazenadas](docs/credenciaisarmazenadas.png)
![Credenciais Armazenadas 1](docs/credenciaisarmazenadasN.png)
*Armazenamento de credenciais capturadas durante a operação do sistema*

### 5. Análise de Segurança
![Análise de Segurança](docs/analisedeseguranca.png)
*Relatório de análise de segurança das redes WiFi detectadas*

### 6. Integração com Google Sheets
![Exemplo Planilha](docs/exemploplanilha.png)
*Dados sincronizados com Google Sheets para análise posterior*

### 7. Configuração WiFi ESP32
![CfWiFi](docs/cfwifi.png)
![CfWiFi1](docs/cfwifi1.png)
![CfWiFi2](docs/cfwifi2.png)
![CfWiFi3](docs/cfwifi3.png)
![CfWiFi4](docs/cfwifi4.png)
*Portal Configurar o WiFi do ESP32*

![ConfigurarWifiESP32](docs/configurarwifiesp32.png)
*Configurar o WiFi do ESP32*


## 🎓 Objetivos Educacionais

Este projeto ensina conceitos fundamentais de:

- **Protocolos WiFi**: 802.11, WPA/WPA2/WPA3
- **Vulnerabilidades**: Redes abertas, WEP, Evil Twins
- **Técnicas de Ataque**: Portal captivo, DNS hijacking
- **Medidas de Proteção**: Configurações seguras
- **Análise de Riscos**: Identificação e mitigação
- **Ethical Hacking**: Princípios e responsabilidades

**⚠️ Lembre-se**: Use este projeto responsavelmente e apenas para fins educativos!
