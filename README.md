# Tarefa: Roteiro de FreeRTOS #1 - EmbarcaTech 2025

- Autor: **Guilherme Westphall**
- Curso: Residência Tecnológica em Sistemas Embarcados
- Instituição: EmbarcaTech - HBr

📍 Brasília, 13 de Junho de 2025

---

# FreeRTOS na BitDogLab

> Atividade prática de introdução ao FreeRTOS, com foco na utilização do FreeRTOS no RP2040.

## Objetivo 🎯

Criar e gerenciar múltiplas tasks utilizando o FreeRTOS no microcontrolador RP2040 embutido na [BitDogLab](https://github.com/BitDogLab/BitDogLab). Compreender o escalonamento de tarefas, prioridades e sincronização entre tarefas. Usar funções básicas do FreeRTOS, como `xTaskCreate`, `vTaskDelay`, `vTaskSuspend` e `vTaskResume`. 

## Componentes Utilizados 🛠️

| Componente | Descrição                                 | Pinagem                          |
| ---------- | ----------------------------------------- | -------------------------------- |
| BitDogLab  | Placa de desenvolvimento com RP2040       | -                                |
| LED RGB    | Alternando entre Vermelho, Verde e Azul   | GPIO11 (G) GPIO12 (B) GPIO13 (R) |
| Buzzer     | Emite um *beep* periodicamente            | GPIO21                           |
| Botão A    | Para suspender/retomar a tarefa dos LEDs  | GPIO5                            |
| Botão B    | Para suspender/retomar a tarefa do Buzzer | GPIO6                            |
| Display OLED | Exibe o estado atual das tasks | GPIO14 (SDA) GPIO15 (SCL) |

## Estrutura do Projeto 📂

```
tarefa-freertos-1-west7/
│
├── CMakeLists.txt            # Configuração do CMake
├── README.md                 # Documentação do projeto
├── LICENSE                   # Licença do projeto
├── main.c                    # Código principal
├── inc/                      # Diretório de cabeçalhos    
│   ├── buzzer.h              # Header e implementação do Buzzer
│   └── FreeRTOSConfig.h      # Configurações do FreeRTOS
├── FreeRTOS/                 # FreeRTOS Kernel
│   └── ...
```

## Relatório 

### O que acontece se todas as tarefas tiverem a mesma prioridade?

Quando todas as tarefas têm a mesma prioridade, o escalonador do FreeRTOS utiliza um escalonamento circular [Round-Robin](https://west7.github.io/FSO/notes/04_gerencia_do_processador/#422-round-robin-escalonamento-circular) para alternar entre elas. Isso significa que cada tarefa terá uma fatia de tempo igual para ser executada, permitindo que todas tenham a chance de rodar. Apesar de justo, o principal problema deste algoritmo é: se a fatia de tempo (ou *quantum*) de cada tarefa for muito curta, pode haver um overhead significativo devido à troca frequente de contexto entre as tarefas. Isso pode levar a uma redução no desempenho geral do sistema. 

Outro ponto importante é que, como as tarefas desta atividade são relativamente simples e rápidas, o efeito do escalonamento circular não é tão perceptível. No entanto, em sistemas mais complexos, isso poderia levar a um aumento no tempo de resposta e na latência (queda de desempenho significativa).


### Qual tarefa consome mais tempo de CPU?

A tarefa que consome mais tempo de CPU é aquela que executa operações mais demoradas ou que tem um loop contínuo sem delays significativos. No caso deste projeto, a tarefa que se encaixa com essa descrição é a task dos botões, pois ela está quase que constantemente verificando o estado dos botões e realizando ações com base nisso, a não ser por um pequeno delay de 100ms.

Em contrapartida, as duas outras tarefas (LEDs e Buzzer) têm delays mais longos, o que reduz o tempo de CPU consumido por elas. A tarefa dos LEDs alterna entre vermelho, verde e azul a cada 500ms, enquanto a tarefa do Buzzer emite um beep a cada 1 segundo.

Quando `vTaskDelay` é utilizado, a tarefa entra em estado de bloqueio, fazendo com que o escalonador selecione outra tarefa que está pronta para ser executada. Isso permite que outras tarefas tenham a chance de rodar, evitando que uma única tarefa monopolize a CPU.   

### Qual seria o risco de usar polling sem prioridades?

O maior risco de usar polling sem prioridades é o monopólio da CPU por uma única tarefa. A tarefa de polling, ou seja, verificar algo constantemente, é uma tarefa que nunca cede o processador, o que causaria "starvation" para outras tarefas de mesma prioridade. No código, caso eu coloque a task dos botões (faz polling) com uma prioridade maior que as outras, em um primeiro momento, nada seria afetado. Porém, caso eu removesse a linha do delay:


```c
void button_task(void *params)
{
   ...
    //vTaskDelay(pdMS_TO_TICKS(100));
}

xTaskCreate(led_task, "LED_Task", 256, NULL, 1, &led_task_handle);
xTaskCreate(buzzer_task, "Buzzer_Task", 256, NULL, 1, &buzzer_task_handle);
xTaskCreate(button_task, "Button_Task", 256, NULL, 2, NULL); // Prioridade maior
```
O que aconteceria seria que a task dos botões ficaria em um loop infinito, consumindo toda a CPU e impedindo que as outras tarefas (LEDs e Buzzer) fossem executadas. Isso levaria a um sistema não responsivo, onde os LEDs não piscariam e o Buzzer não emitiria sons, pois a tarefa de polling estaria sempre rodando.

### Usando Semaphores (Extra)

Ao usar semáforos, a sincronização é feita de maneira eficiente, contudo existem algumas diferenças importantes em comparação ao uso de `vTaskSuspend`: ao suspender a tarefa, ela é retirada da fila de execução, ou seja, ela é ignorada pelo escalonador até que seja retomada. Já com semáforos, a tarefa permanece na fila de execução, mas fica bloqueada até que o semáforo seja liberado (polling). 

Os semáforos são mais flexíveis e permitem uma comunicação mais eficiente entre tarefas, especialmente em sistemas com múltiplas tarefas que precisam compartilhar recursos ou sincronizar ações. Em contraste, `vTaskSuspend` é mais simples e pode ser útil em casos onde uma tarefa precisa ser completamente interrompida e retomada posteriormente, mas não serve para proteger recursos específicos ou sinalizar eventos.

A suspensão de tarefas é um evento que vêm de fora, isso implica que a tarefa não tem a chance de "se preparar" para ser suspensa (e.g. liberar um recurso, salvar um estado, etc.) isso pode levar a comportamentos inesperados, além de que o fluxo de execução pode ser mais difícil de prever. Isso pode ser claramente visto no código, onde com o uso de `vTaskSuspend`, a tarefa dos LEDs e do Buzzer são suspensas exatamente no estado em que estão, exatamente no momento em que o botão é pressionado, isto é, se o LED estava vermelho, ele será suspenso nesse estado e não terá a chance de mudar para verde ou azul. Enquanto que com semáforos, a tarefa sempre fechará o ciclo de execução antes de ser bloqueada, sempre parando no estado final de execução, ou seja, se o LED estava vermelho, ele irá mudar para verde e depois azul antes de ser bloqueado. Claro que isso depende de como a tarefa é implementada, eu poderia "forçar" o estado final da tarefa, caso isso fosse necessário.

Evidentemente, o uso de semáforos é mais adequado para sistemas embarcados, onde a eficiência e a previsibilidade são cruciais. Eles permitem uma melhor gestão de recursos e evitam problemas como deadlocks e starvation, que podem ocorrer com o uso inadequado de `vTaskSuspend`.

## Execução 🧪

1. Faça o clone do projeto:

```bash
git clone https://github.com/EmbarcaTech-2025/tarefa-freertos-1-west7.git
```

2. Compile e Embarque o firmware na Raspberry:

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
cp embarcatech-freertos-tarefa-1 .uf2 /media/$USER/RPI-RP2/
```