
Named group of users. 

 *Channels names are strings (beginning with a '&', '#', '+' or '!' character) of length up to fifty (50) characters.  Channel names are case insensitive.* 

The use of different prefixes effectively creates four (4) distinct
   namespaces for channel names


   Channels with '&' as prefix are local to the server where they are
   created.

|Concepto|Descripción|
|---|---|
|**Canal**|Grupo nombrado de usuarios. Todos los mensajes dirigidos a ese canal se reenvían a sus miembros.|
|**Nombre del canal**|Empieza por `#`, `&`, `+` o `!`. En tu proyecto basta con `#`. No puede contener espacios, comas ni `^G`.|
|**Creación**|El canal se crea automáticamente cuando el primer usuario hace `JOIN #nombre` y desaparece cuando el último usuario sale.|
|**Servidor responsable**|Cada canal solo existe dentro del conjunto de servidores que lo conocen. En tu implementación (un solo servidor) no necesitas propagar nada.|
