#!/bin/bash

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Funções para logging
error() {
    echo -e "${RED}[ERRO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCESSO]${NC} $1"
}

info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

# Função para verificar se o comando anterior foi bem-sucedido
check_error() {
    if [ $? -ne 0 ]; then
        error "$1"
        exit 1
    fi
}

# Função para verificar dependências
check_dependencies() {
    info "Verificando dependências..."
    
    # Verifica se o gcc está instalado
    if ! command -v gcc &> /dev/null; then
        error "GCC não encontrado. Instalando..."
        sudo apt install -y gcc
        check_error "Falha ao instalar GCC"
    else
        success "GCC já instalado"
    fi
    
    # Verifica se make está instalado
    if ! command -v make &> /dev/null; then
        info "Make não encontrado. Instalando..."
        sudo apt install -y make
        check_error "Falha ao instalar make"
    fi
}

# Função para limpeza em caso de erro
cleanup() {
    if [ -f "bearbox" ]; then
        rm -f bearbox
        info "Arquivos temporários removidos"
    fi
}

# Configurar trap para limpeza em caso de erro
trap cleanup EXIT

clear
echo "=========================================="
echo "  Instalador do Bearbox v.beta"
echo "=========================================="
echo ""

# Verificar se é executado como root
if [ "$EUID" -eq 0 ]; then 
    error "Não execute este script como root/sudo. Execute como usuário normal."
    exit 1
fi

# Verificar se o arquivo bear.c existe
if [ ! -f "bear.c" ]; then
    error "Arquivo bear.c não encontrado no diretório atual!"
    error "Certifique-se de que o arquivo bear.c está no mesmo diretório deste instalador."
    exit 1
fi

info "Iniciando instalação do Bearbox..."

# Atualizar repositórios
info "Atualizando lista de pacotes..."
sudo apt update
check_error "Falha ao atualizar lista de pacotes"

# Atualizar sistema (opcional, pode ser comentado se não quiser atualizar tudo)
info "Atualizando sistema..."
sudo apt upgrade -y
if [ $? -ne 0 ]; then
    error "Atenção: Houve problemas ao atualizar o sistema"
    read -p "Deseja continuar mesmo assim? (s/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Ss]$ ]]; then
        exit 1
    fi
fi

# Verificar e instalar dependências
check_dependencies

# Instalar bibliotecas necessárias
info "Instalando bibliotecas necessárias..."
sudo apt-get install -y libncurses5-dev libncursesw5-dev
check_error "Falha ao instalar bibliotecas ncurses"

# Compilar o programa
info "Compilando Bearbox..."
gcc bear.c -o bearbox -lncurses -Wall -Wextra
check_error "Falha ao compilar bear.c. Verifique se o código-fonte está correto."

# Verificar se o binário foi criado
if [ ! -f "bearbox" ]; then
    error "Binário bearbox não foi criado durante a compilação"
    exit 1
fi

# Testar o executável
info "Testando o executável..."
./bearbox --help 2>/dev/null || ./bearbox -h 2>/dev/null || echo "Executável criado, testando funcionalidade básica..."
if [ $? -ne 0 ]; then
    warning "O executável pode não estar funcionando corretamente"
fi

# Mover para /usr/local/bin
info "Instalando em /usr/local/bin..."
sudo mv bearbox /usr/local/bin/
check_error "Falha ao mover bearbox para /usr/local/bin"

# Configurar permissões
sudo chmod 755 /usr/local/bin/bearbox
check_error "Falha ao configurar permissões"

# Verificar instalação
info "Verificando instalação..."
if command -v bearbox &> /dev/null; then
    success "Bearbox instalado com sucesso!"
    
    # Mostrar informações da instalação
    echo ""
    echo "=========================================="
    echo "  Instalação concluída!"
    echo "=========================================="
    echo ""
    echo "Bearbox foi instalado em: /usr/local/bin/bearbox"
    echo ""
    echo "Para executar, use: bearbox"
    echo "Para ver a ajuda, use: bearbox --help"
    echo ""
    
    # Oferecer para executar o programa
    read -p "Deseja executar o Bearbox agora? (s/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Ss]$ ]]; then
        echo "Executando Bearbox..."
        echo "=========================================="
        bearbox
    fi
else
    error "Houve um problema na instalação. Bearbox não está acessível no PATH."
    exit 1
fi

exit 0

