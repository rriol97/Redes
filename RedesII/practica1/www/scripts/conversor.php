#!/usr/bin/php
<?php
/***************************************************************************
* conversor.php
*
* Script que recibe una temperatura en grados Celsius y la devuelve
* en Farenheit.
*
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/
$dirFichero = $argv[2];
$file = fopen ($dirFichero, "w");
if ($argc != 3) {
	fwrite($file, "Error, se debe introducir solo dos argumento de entrada". PHP_EOL);
} else {
	$campos = explode("=", $argv[1]);
	if ((count($campos) != 2) || (strcmp($campos[0], "temp"))) {
		fwrite($file, "Error, mal introducidos los argumentos: temp=valor". PHP_EOL);
	} else {
		$campos = explode(" ", $campos[1]);
		fwrite($file, "Grados Farenheit: ". (($campos[0])*1.8+32). PHP_EOL);
	}
}
fclose($file);
?>

