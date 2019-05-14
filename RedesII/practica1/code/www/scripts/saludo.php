#!/usr/bin/php
<?php
/***************************************************************************
* saludo.php
*
* Script que recibe un nombre y responde con la cadena "Hola <nombre>!"
*
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/
$file = fopen($argv[2], "w");
if ($argc != 3) {
	fwrite($file, "Error, se deben introducir solo dos argumento de entrada". PHP_EOL);
} else {
	$campos = explode("=", $argv[1]);
	if ((count($campos) != 2) || (strcmp($campos[0], "nombre"))) {
		fwrite($file, "Error, mal introducidos los argumentos: nombre=valor". PHP_EOL);
	} else {
		fwrite($file, "Hola");
		$separados = explode("+", $campos[1]);
		for ($i=0, $num = count($separados); $i < $num; $i++) {
			fwrite($file, " ". $separados[$i]);
		}
		fwrite($file, "!". PHP_EOL);
	}
}
fclose($file);
?>

