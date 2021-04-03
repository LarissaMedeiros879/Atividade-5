/* Define a frequencia do clock. */
#define F_CPU 16000000UL 

/* Bibliotecas necessarias para o funcionamento do programa. A primeira e uma biblioteca basica e a segunda permite o uso do delay. */
#include <avr/io.h>
#include <util/delay.h>

unsigned char *mcucr; /* MCUCR e o registrador de controle. */
unsigned char *ubrr0h; /* UBRR0X sao registradores para a definicao de baud rate, que e a velocidade com que a transmissao e feita. */
unsigned char *ubrr0l; /* UBRR0X sao registradores para a definicao de baud rate, que e a velocidade com que a transmissao e feita. */
unsigned char *ucsr0a; /* UCSR0X sao registradores de configuracao e status da USART */
unsigned char *ucsr0b; /* UCSR0X sao registradores de configuracao e status da USART */
unsigned char *ucsr0c; /* UCSR0X sao registradores de configuracao e status da USART */
char *udr0; /* Ponteiro do registrador de dados da USART0. Esta sendo definido como char pois recebera as strings de mensagem definidas como char. */
unsigned char *p_ddrd; /* Ponteiro do registrador port d. O bit 1 desse registrador esta associado ao pino TXD. */
unsigned char *p_portb; /* Ponteiro dos registradores Port B */
unsigned char *p_ddrb; /* Ponteiro dos registradores Port B */
unsigned char *p_pinb; /* Ponteiro dos registradores Port B */

char msg_np[] = "Transmissao serial utilizando a USART: verificacao do termino da transmissao por varredura. O botao nao esta pressionado.\n\n"; /* Mensagem que deve ser exibida no monitor caso o botao nao esteja pressionado. */
char msg_p[] = "Transmissao serial utilizando a USART: verificacao do termino da transmissao por varredura. O botao esta pressionado.\n\n"; /* Mensagem que deve ser exibida no monitor caso o botao esteja pressionado. */
char *np = &(msg_np[0]); /* Ponteiro que recebe o endereco do primeiro caractere da mensagem de botao nao pressionado. */
char *p = &(msg_p[0]); /* Ponteiro que recebe o endereco do primeiro caractere da mensagem de botao pressionado. */

/* Funcao que configura os perifericos. */
void setup() {
  /* Atribui ao ponteiro o endereco do regristrador de controle. Ponteiro faz o controle de pull up do arduino, permitindo o uso do botao. */ 
  mcucr = (unsigned char *)0x55;
  /* Quando tem 1 no bit 4, desabilita o pull up, do contrario, habilita */
  *mcucr &= ~(1 << 4);
  /* Atribui ao ponteiro o endereco do registrador de dados da USART0. */
  udr0 = (char *) 0xC6;
  /* Atribui aos ponteiros os endereços dos registradores UBRR0X. */
  ubrr0h = (unsigned char *) 0xC5;
  ubrr0l = (unsigned char *) 0xC4;
  /* Para um baud rate de 115.2 kbps, UBRR0X = 8. Em binario, 0000 0000 0000 1000. Os oito primeiros bits correspondem ao UBRR0H e os oito ultimos ao UBRR0L.  */
  *ubrr0h = 0b00000000;  /* Aqui, todos os bits estao sendo usados. Por isso esta sendo usado o = ao inves de uma mascara. */
  *ubrr0l = 0b00001000;  /* Aqui, todos os bits estao sendo usados. Por isso esta sendo usado o = ao inves de uma mascara. */
  /* Atribi aos ponteiros os endereços dos registradores UCSR0X. */
  ucsr0a = (unsigned char *) 0xC0; 
  ucsr0b = (unsigned char *) 0xC1;
  ucsr0c = (unsigned char *) 0xC2;
  /* Os bits do registrador UCSR0A correspondem a: 
  7 - RXCO: uma flag que indica que existem dados nao lidos no buffer de recepcao. Como o programa envolve apenas uma transmissao, ele nao sera usado. 
  6 - TXCO: uma flag setada quando todo o dado de transmissao foi enviado. E usada ao longo do processo de transmissao. Inicialmente, nao esta setada. 
  5 - UDRE0: flag que indica se o buffer de transmissao esta pronto para receber novos dados. E um bit a ser lido, logo nao e setado inicialmente. 
  4, 3, 2 - FE0, DOR0, UPE0: flas que indicam, respectivamente, erro no frame recebido, overrun e erro de paridade. Tambem sao bits a serem lidos e nao sao setados inicialmente.
  1 - U2X0: quando igual a 1, a taxa de transmissao e dobrada. Como se quer uma velocidade de transmissao normal, ele e setado como 0.
  0 - MPCM0: ativa o modo multiprocessador. Como se deseja desabilita-lo, o bit e setado como 0. */
  *ucsr0a &= 0xFC;
  /* Os bits do registrador UCSR0B correspondem a:
  7, 6, 5 - RXCIE0, TXCIE0, UDRIE0: esses bits habilitam/desabilitam interrupcoes. Como se deseja desabilitar todas as interrupcoes, os bits sao todos setados como zero
  4 - RXEN0: habilia/desabilita o receptor, logo e setado como zero para desabilita-lo, ja que o programa envolve apenas transmissao. 
  3 - TXEN0: habilia/desabilita o transmissor, logo e setado como um para habilita-lo, ja que o programa envolve uma transmissao.
  2 - UCSZ02: esse bit, em conjunto com UCSZ01 e UCSZ00 selecionam quantos bits de dados serao transmitidos em cada frame. Como sao 8 bits, UCSZ0X e setado como 011. Logo, esse bit e setado como zero. 
  1, 0 - RXB80, TXB80: sao usados quando se trata de uma transmissao com 9 bits. Logo, nao sao utilizados nessa transmissao. */
  *ucsr0b &= 0xB; /* Reseta os bits que devem ser iguais a 0. */
  *ucsr0b |= 0x08; /* Seta o bit que deve ser igual a 1. */
  /*Os bits do registrador UCSR0C correspondem a:
  7, 6 - UMSEL01, UMSEL00: definem o modo de operação da USART. Para que ela funcione em modo assincrono, os bits devem ser setados como 00.
  5, 4 - UPM01, UPM00: definem o uso (ou não) do bit de paridade e o seu tipo. Aqui sera usado um bit de paridade par, logo os bits sao setados como 10.
  3 - USBS0: esta relacionado aos bits de parada. E igual a 0 quando há um único bit de parada em cada frame e igual a 1 quando dois bits de parada são utilizados. Nessa transmissao, e setado como 1.
  2, 1 - UCSZ01, UCSZ00: esses bits, em conjunto com UCSZ02 selecionam quantos bits de dados serao transmitidos em cada frame. Como sao 8 bits, UCSZ0X e setado como 011. Logo, esses bits sao setados como 11.
  0 - UCPOL0: deve ser igual a zero para transmissao assíncrona.*/
  *ucsr0c = 0b00101110; /* Aqui, todos os bits estao sendo usados. Por isso esta sendo usado o = ao inves de uma mascara. */
  /* Atribui aos ponteiros os enderecos do registrador Port D. */
  p_ddrd = (unsigned char *) 0x2A;
  /* O bit 1 do registrador esta associado ao pino TDX. Para uma transmissao, ele dever ser configurado como uma saida, ou seja, setado em 1. */
  *p_ddrd |= 0x2;
  /* Atribui aos ponteiros os enderecos do registrador Port B */
  p_portb = (unsigned char *) 0x25;
  p_ddrb = (unsigned char *) 0x24;
  p_pinb = (unsigned char *) 0x23;
  /* O pino PB1 e onde sera conectado o botao, logo deve-se configurar o registrador portb como uma entrada no bit 1. */
  *p_ddrb &= 0xFD;
  /* E necessario ativar o resistor de pull up em PB1 para o acionamento do botao. */
  *p_portb |= 0x02;
}

/* Funcao que configura a transmissao de caracteres para o registrador de dados udr0. */
void transmissao()
{
      /* Verifica o estado do botao. Caso esteja apertado, o bit 1 do registrador pinb estara setado em 0. */
      if ((*p_pinb & 0x02) == 0) {
        /* Se o ponteiro que aponta para os enderecos do vetor msg_p e diferente de '\0', significa que ainda nao chegou ao fim da linha. Assim, contiua-se a transmissao. */
        if (*p != '\0') {
          /* Transmito os caracteres para o registrador de dados UDR0. */
          *udr0 = *p;
          /* Incremento o ponteiro para que ele aponte para o proximo caractere da mensagem. */
          p++;
        }
        else { /* Indica que *p = '\0', ou seja, toda a mensagem foi transmitida. */
          /* Como toda a mensagem foi transmitida, o ponteiro volta a apontar para o primeiro caratere da mensagem para que ela possa ser transmitida novamente. */
          p = &(msg_p[0]); 
          /* Um delay de 500 ms segundos e inserido para que a proxima transmissao ocorra apenas depois desse intervalo de tempo. */
          _delay_ms(500);
        }
      }
      else { /* Caso em que o botao nao esta pressionado */
        /* Se o ponteiro que aponta para os enderecos do vetor msg_np e diferente de '\0', significa que ainda nao chegou ao fim da linha. Assim, contiua-se a transmissao. */
        if (*np != '\0') {
          /* Transmito os caracteres para o registrador de dados UDR0. */
          *udr0 = *np;
          /* Incremento o ponteiro para que ele aponte para o proximo caractere da mensagem. */
          np++;
        }
        else { /* Indica que *np = '\0', ou seja, toda a mensagem foi transmitida. */
          /* Como toda a mensagem foi transmitida, o ponteiro volta a apontar para o primeiro caratere da mensagem para que ela possa ser transmitida novamente. */
          np = &(msg_np[0]);
          /* Um delay de 500 ms segundos e inserido para que a proxima transmissao ocorra apenas depois desse intervalo de tempo. */
          _delay_ms(500);
        }
      }
}

int main(){
  /* Inicializa os perifericos */
  setup();

 /* Looping infinito. */
  while (1) {
    /* Enquanto o buffer de transmissao estiver vazio, executa a transmissao. Isso e verificado atraves do quinto bit de ucsr0a, o qual corresponde a flag udre0. Quando esta esta setada em 1, o buffer de transmissao esta vazio. Caso contrario, nao pode receber dados. */
    while ((*ucsr0a & 0b00100000) == 0b00100000) {
      /* Enquanto o buffer esta vazio, faz a transmissao de caracteres. */
      transmissao();
    }
    
  } 
  }