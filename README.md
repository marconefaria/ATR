# ATR

Uma empresa petrolífera acaba de instalar um sistema de otimização de gas lift em uma de suas plataformas offshore.  Este sistema adquire, do Controlador Lógico Programável (CLP) que controla as válvulas de gas lift, valores das  variáveis de processo relevantes como pressão e temperatura no tubo de extração, nível e pressão do reservatório  de gás, etc., para, em resultado, determinar o volume e pressão do gás a ser injetado no tubo de extração. 
A empresa deseja integrar as informações deste sistema de otimização com aquelas já existentes no sistema  SCADA (Supervisory Control and Data Acquisition) da plataforma petrolífera, direcionando todas estas para três  painéis de projeção de imagens presentes na sala de controle da plataforma, os quais apresentam respectivamente  dados do sistema de otimização, dados do processo de extração do óleo cru e mensagens de alarmes industriais.  Para tal, você foi contratado para desenvolver uma aplicação de software multithread responsável pela leitura de  dados do sistema de otimização e do sistema SCADA, e distribuí-los adequadamente para os três painéis de  projeção. Os dados do sistema de otimização chegam em intervalos variáveis de 1 a 5 segundos, ao passo que os  dados do sistema SCADA devem ser lidos periodicamente do SDCD a cada 500 ms. A aplicação a ser  desenvolvida deverá ser composta pelas seguintes tarefas (onde o termo “tarefa” pode designar tanto processos  quanto threads):  

  1. Tarefa de comunicação de dados. Executa a leitura de dados do sistema de otimização e do sistema  SCADA, depositando-os em uma lista circular em memória.  
  2. Tarefa de retirada de dados de otimização. Esta tarefa retira, da lista circular em memória, as mensagens  de dados do sistema de otimização e as deposita em um arquivo circular em disco com capacidade para 200  mensagens.   PARTE 2
  3. Tarefa de retirada de dados de processo. Esta tarefa retira, da lista circular em memória, as mensagens do  sistema SCADA correspondentes a dados de processo e as repassa para a tarefa de exibição de dados de  processo (vide abaixo).  
  4. Tarefa de retirada de alarmes. Esta tarefa retira, da lista circular em memória, as mensagens do sistema  SCADA correspondentes a alarmes e as repassa para a tarefa de exibição de alarmes (vide abaixo).  
  5. Tarefa de exibição de dados de otimização. Esta tarefa retira mensagens do arquivo circular em disco e as  exibe no respectivo painel de projeção de vídeo, na sala de controle.   PARTE 2
  6. Tarefa de exibição de dados de processo. Esta tarefa recebe mensagens de dados de processo e as exibe no  respectivo painel de projeção de vídeo, na sala de controle.  
  7. Tarefa de exibição de alarmes. Esta tarefa recebe mensagens de alarmes do sistema e as exibe no respectivo  painel de projeção de vídeo, na sala de controle.  
  8. Tarefa de leitura do teclado. Esta tarefa dá tratamento aos comandos digitados pelo usuário.  

# ETAPA 1 – Arquitetura multitarefa/multithread e implementação da lista circular em memória

Nesta etapa será elaborada a arquitetura da aplicação, seguida do desenvolvimento e teste de partes de suas  funcionalidades:  
  ▪ Arquitetura multitarefa/multithread da aplicação. Em outras palavras, corresponde à definição de como as  tarefas descritas serão modeladas em termos de processos e threads;  
  ▪ Disparo da aplicação, com a consequente execução de seus processos e threads;  
  ▪ Implementação do mecanismo de bloqueio/desbloqueio das threads e de seus encerramentos, mediante um  conjunto de objetos do tipo evento;  
  ▪ Implementação da lista circular em memória, com o depósito de mensagens na mesma pela tarefa de  comunicação de dados, e a retirada das mesmas pelas tarefas de retirada de mensagens, levando em conta aspectos de sincronização eventualmente necessários;  
  ▪ Temporizações provisórias da tarefa de comunicação de dados, por meio da função Sleep() com periodicidade  única de 1000 ms.  
Leve em conta os seguintes aspectos nesta etapa do trabalho:  
  1. Apesar de o programa corresponder a mais de um arquivo executável, lembre-se que, do ponto de vista do usuário, seu disparo deve ocorrer a partir de um único arquivo executável. Em outras palavras, você deve  definir um processo principal a partir do qual os demais processos serão disparados por meio da função CreateProcess() da API Win32.  
  2. Na API Win32, um processo criado pode ter sua própria janela de console ou compartilhar a janela de  console do processo criador. Estude a função CreateProcess() para entender como utilizar um ou outro caso.  3. O manuseio de uma lista circular em memória está descrito na seção “O problema dos produtores e consumidores” do livro “Programação Concorrente em Ambiente Windows”. Note que, no caso da primeira  lista circular em memória, há uma tarefa “produtora” e três “consumidoras” que acessam a lista. Assim, cada  uma destas tarefas deverá manter e manipular apontadores individuais para a próxima mensagem a ser  depositada ou retirada. Outra opção é implementar esta lista circular como uma lista encadeada (linked list).  
Para fins de certificação de conclusão desta etapa, a tarefa de retirada de mensagens deverá exibir as mensagens  retiradas na console principal, bem como seu estado de bloqueio/desbloqueio. As tarefas de exibição de dados  deverão apenas apresentar uma mensagem simples em suas respectivas janelas de console, informando os seus  estados de bloqueio/desbloqueio. O diagrama de relacionamentos na Figura 2 apresenta a estrutura da aplicação correspondente à primeira etapa.  

# ETAPA 2 – Temporização, IPC e conclusão da aplicação  

Nesta etapa a aplicação será finalizada, com o acréscimo das seguintes funcionalidades:  ▪ Temporizações de depósito de mensagens na lista circular em memória pela tarefa de comunicação de dados,  nas periodicidades indicadas na descrição da mesma; 
  ▪ Criação do arquivo circular em disco e inserção/retirada no/do mesmo das mensagens de dados do sistema de  otimização;  
  ▪ Implementação dos mecanismos de IPC (Inter-Process Communication) entre as tarefas de retirada de mensagens e  as respectivas tarefas de exibição de dados, conforme especificado em suas descrições;  ▪ Testes finais da aplicação como um todo. (Atenção: seja cuidadoso e minucioso em seus testes. Procure testar  o máximo de situações válidas e inválidas possíveis no funcionamento da aplicação. O grande cientista da  computação Niklaus Wirth – criador, entre outras grandes contribuições, da linguagem Pascal – afirmava que  testes apenas provam a presença de erros, nunca a ausência destes.) 
O diagrama de relacionamentos na Figura 3 apresenta a estrutura da aplicação correspondente à segunda etapa. 