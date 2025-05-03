# Sistema de Controle de Semáforo Inteligente

Este projeto implementa um sistema de controle de semáforo inteligente utilizando o microcontrolador RP2040 da BitDogLab. O sistema gerencia sinais visuais e sonoros para indicar os estados do semáforo (verde, amarelo, vermelho) e alterna entre modos de operação (normal e noturno). Ele utiliza multitarefa com FreeRTOS para controlar LEDs RGB, matriz de LEDs WS2812B, display OLED e buzzer.

## Funcionalidades

- Modos de Operação:
  - Modo Normal:
    - Alterna entre os estados do semáforo: verde, amarelo e vermelho.
    - LEDs RGB, matriz de LEDs e buzzer sincronizados com o estado atual.
  - Modo Noturno:
    - Semáforo fixo no estado amarelo piscando.
    - Buzzer emite tom grave e intermitente.
    - Reduz impacto visual e sonoro durante a noite.
- Controle de LEDs RGB:
  - Indicam visualmente o estado do semáforo.
  - Verde: Pode atravessar.
  - Amarelo: Atenção.
  - Vermelho: Pare.
- Matriz de LEDs WS2812B:
  - Representa o estado do semáforo com cores configuráveis.
  - No modo noturno, exibe o LED amarelo piscando.
- Display OLED:
  - Exibe o modo atual do sistema ("Modo Normal" ou "Modo Noturno").
  - Mostra mensagens como "Pode Atravessar", "Atenção!" e "Pare!" dependendo do estado.
- Buzzer:
  - Emite sons distintos para cada estado no modo normal.
  - No modo noturno, emite um tom grave e intermitente.
- Botões:
  - Botão A: Alterna entre os modos normal e noturno.
  - Botão B: Reinicia o sistema no modo BOOTSEL.

## Hardware Utilizado

- Microcontrolador: RP2040 (BitDogLab).
- Periféricos:
  - LEDs RGB.
  - Matriz de LEDs WS2812B.
  - Display OLED SSD1306.
  - Buzzer.
  - Botões (A e B).

## Como executar o projeto

Clone o Repositório:

```bash
git clone https://github.com/matheusssilva991/tarefa3_multitarefa_embarcatech.git
cd tarefa3_multitarefa_embarcatech
```

Importe o projeto no VS Code utilizando a extensão Raspberry Pi Pico

- Abra o Visual Studio Code.
- Instale a extensão Raspberry Pi Pico (se ainda não estiver instalada).
- Importe o projeto clicando na extensão e em import project

Configure o Caminho do FreeRTOS no CMakeLists.txt:

- Abra o arquivo CMakeLists.txt na raiz do projeto.
- Localize a linha onde o FreeRTOS é incluído e ajuste o caminho para - apontar para a pasta onde o FreeRTOS está localizado no seu sistema:

```bash
set(FREERTOS_PATH "caminho/para/freertos/source")
```

Compile o Projeto:

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
```

## Link da demonstração

[Link para o vídeo de demonstração](https://www.drive.com)

## Licença

Este projeto é de código aberto e pode ser utilizado e modificado livremente para fins educacionais e não comerciais.