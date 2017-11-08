BEGIN{suma=0;tot_lin=0}
{
  if(($1==2048) || ($2==2048)){suma+=1}
  tot_lin+=1
}
END{print"El porcentaje de paquetes ip es", suma/tot_lin*100"%"}
