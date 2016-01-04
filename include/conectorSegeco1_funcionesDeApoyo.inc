<?php


function crearPDF($nid, $pdf1 = NULL) {
    module_load_include('inc', 'print_pdf', 'print_pdf.pages'); 
    $path = 'printpdf/node/' . $nid . '/paper-work'; //Aqui establecemos muestra e informe
    $pdf2= print_pdf_generate_path($path);
//    print_r($pdf2);
    include libraries_get_path('PDFMerger') . 'PDFMerger.php';

    $pdf = new PDFMerger;

    $pdf->addPDF($pdf2, 'all')
            ->addPDF($pdf2, 'all')
            ->merge('file', 'TEST2.pdf');
	
    
}



function conectorSegcan1_crearArrayDeVial($node) {
    //vamos a ver de que ayntamiento estamos.
    $ayto = NULL;
    $config = variable_get('codeserver_configs');
    foreach ($config as $conectorName => $server) {
        $ayto = str_replace('conector', '', $conectorName);
    }
    //Vamos a ver que tipo de vial es.
    $v = node_load($node->field_solicitud_codigovial['und'][0]['target_id']);
    
    
    $vial['tipoDeVial']         = $v->field_vial_tipo_de_vial['und'][0]['value'];
    $vial['titulo']             = isset($node->field_solicitud_codigovial['und'][0]['target_id']) ? conectorSegeco1_getNodeById($node->field_solicitud_codigovial['und'][0]['target_id']) : NULL;
    $vial['fechaVialLleno']     = date('d-m-Y h:i', $node->created);  
    $vial['ayuntamiento']       = $ayto;
    $vial['veterinario']        = conectorSegeco1_getUserById($node->uid);
    $vial['tituloSolicitud']    = $node->title;  
    $vial['tipoChapa']          = isset($node->field_perro_tipo_de_chapa['und'][0]['value']) ? $node->field_perro_tipo_de_chapa['und'][0]['value'] : NULL;    
    $vial['nombrePerro']        = isset($node->field_perro_nombre_ejemplar['und'][0]['value']) ? $node->field_perro_nombre_ejemplar['und'][0]['value'] : NULL;    
    $vial['digitoControl']      = isset($node->field_perro_digito_de_control['und'][0]['value']) ? $node->field_perro_digito_de_control['und'][0]['value'] : NULL;   
    $vial['crearChapa']         = TRUE;
    $vial['tituloChapa']        = $vial['tituloSolicitud'] . ' ' . $vial['nombrePerro'] . ' ' . $vial['digitoControl'];
    
    
    return $vial;
}

/**
 * Función que mete el genotipo en el vial
 * @param object type Vial $v
 * @param array $gen
 */
function conectorSegeco1_setGen($v, $gen) {
    $v->field_marcador_01['und'][0]['value'] = isset($gen['field_marcador_01']) ? $gen['field_marcador_01'] : NULL;
    $v->field_marcador_02['und'][0]['value'] = isset($gen['field_marcador_02']) ? $gen['field_marcador_02'] : NULL;
    $v->field_marcador_03['und'][0]['value'] = isset($gen['field_marcador_03']) ? $gen['field_marcador_03'] : NULL;
    $v->field_marcador_04['und'][0]['value'] = isset($gen['field_marcador_04']) ? $gen['field_marcador_04'] : NULL;
    $v->field_marcador_05['und'][0]['value'] = isset($gen['field_marcador_05']) ? $gen['field_marcador_05'] : NULL;
    $v->field_marcador_06['und'][0]['value'] = isset($gen['field_marcador_06']) ? $gen['field_marcador_06'] : NULL;
    $v->field_marcador_07['und'][0]['value'] = isset($gen['field_marcador_07']) ? $gen['field_marcador_07'] : NULL;
    $v->field_marcador_08['und'][0]['value'] = isset($gen['field_marcador_08']) ? $gen['field_marcador_08'] : NULL;
    $v->field_marcador_09['und'][0]['value'] = isset($gen['field_marcador_09']) ? $gen['field_marcador_09'] : NULL;
    $v->field_marcador_10['und'][0]['value'] = isset($gen['field_marcador_10']) ? $gen['field_marcador_10'] : NULL;
    $v->field_marcador_11['und'][0]['value'] = isset($gen['field_marcador_11']) ? $gen['field_marcador_11'] : NULL;
    $v->field_marcador_12['und'][0]['value'] = isset($gen['field_marcador_12']) ? $gen['field_marcador_12'] : NULL;
    $v->field_marcador_13['und'][0]['value'] = isset($gen['field_marcador_13']) ? $gen['field_marcador_13'] : NULL;
    $v->field_marcador_14['und'][0]['value'] = isset($gen['field_marcador_14']) ? $gen['field_marcador_14'] : NULL;
    $v->field_marcador_15['und'][0]['value'] = isset($gen['field_marcador_15']) ? $gen['field_marcador_15'] : NULL;
    $v->field_marcador_16['und'][0]['value'] = isset($gen['field_marcador_16']) ? $gen['field_marcador_16'] : NULL;
    $v->field_marcador_17['und'][0]['value'] = isset($gen['field_marcador_17']) ? $gen['field_marcador_17'] : NULL;
    $v->field_marcador_18['und'][0]['value'] = isset($gen['field_marcador_18']) ? $gen['field_marcador_18'] : NULL;
    $v->field_marcador_19['und'][0]['value'] = isset($gen['field_marcador_19']) ? $gen['field_marcador_19'] : NULL;
    node_save($v);
}



/**
 * Función que crea un array para meter una fecha estilo
 * @return array[] fecha
 */
function conectorSegeco1_getFecha($fechaSolicitud = NULL, $formato = ' ') {
    //El formato es que si viene vacio le mete un espacio si no le metemos una T
    $fecha = array();
    //Si viene fecha le metemos la que viene si no, le metemos la de ahora.
    $date = $fechaSolicitud ? $fechaSolicitud : date_now('UTC');
    $fecha['value'] = str_replace(" ", $formato, date_format($date, "Y-m-d H:i:s"));
    $fecha['timezone'] = "Europe/Madrid";
    $fecha['timezone_db'] = "Europe/Madrid";
    $fecha['date_type'] = "datetime";
    return $fecha;
}

/**
 * funcion que verifica si un vial está asociado a una solicitud
 * Devuelve false si está lleno
 * devuelve true si está vacio
 */
function conectorSegeco1_checkVialVacio($idVial) {
    $resultado = false;
    $query = db_select('node', 'n');
    $query->join('field_data_field_vial_solicitud', 'fvs', 'n.nid = fvs.entity_id');
    $result = $query
        ->fields('n', array('nid'))
        ->condition('nid', $idVial, '=')
        ->execute();    
    if($result) {       
        while($record = $result->fetchAssoc()) {
            $resultado = TRUE;
        }
    } else {
        $resultado = FALSE;
    }    
    return $resultado;
}


/**
 *  * función que devuelve el nombre del usuario pasandole el id
 */
function conectorSegeco1_getNodeById($id) {
    $query = db_select('node', 'n');
    $result = $query
        ->fields('n', array('title'))
        ->condition('nid', $id, '=')
        ->execute();
    while($record = $result->fetchAssoc()) {
        return $record['title'];
    } 
    return 0;
}

/**
 * función que devuelve un nodo pasandole el título, 
 */
function conectorSegeco1_getNodeByTitle($title){
    $query = db_select('node', 'n');
    $result = $query
        ->fields('n', array('nid'))
        ->condition('title', $title, '=')
        ->execute();
    while($record = $result->fetchAssoc()) {
        return $record['nid'];
    } 
    return 0;
}

/**
 * función que devuelve un nodo pasandole el título, 
 */
function conectorSegeco1_getUserByName($name){
    $query = db_select('users', 'u');
    $result = $query
        ->fields('u', array('uid'))
        ->condition('name', $name, '=')
        ->execute();
    while($record = $result->fetchAssoc()) {
        return $record['uid'];
    } 
    return 0;
}

/**
 *  * función que devuelve el nombre del usuario pasandole el id
 */
function conectorSegeco1_getUserById($id) {
    $query = db_select('users', 'u');
    $result = $query
        ->fields('u', array('name'))
        ->condition('uid', $id, '=')
        ->execute();
    while($record = $result->fetchAssoc()) {
        return $record['name'];
    } 
    return 0;
}

/**
 * función para cambiar datos de forma masiva
 */
function conectorSegeco1_setdatosMasivo() {
    $nidViales = array();
    
    $query = db_select('node', 'n');
    $query->join('field_data_field_perro_estado', 'estado', 'n.nid = estado.entity_id');        
    $result = $query
        ->fields('n', array('nid', 'title'))
        ->fields('estado', array('field_perro_estado_value'))        
        //->condition('nid', 75, '=')
        ->execute();
    while($record = $result->fetchAssoc()) {
        $nidViales[$record['nid']]['estado'] = $record['field_perro_estado_value'];
        $nidViales[$record['nid']]['titulo'] = $record['title'];
    } 
    print_r($nidViales);
    die();
    
    foreach ($nidViales as $key => $idVial) {
        $n = node_load($idVial);
        $estado = $n->field_perro_estado['und'][0]['value'];
        //Vamos a cambiarle los estados
        switch ($estado) {
            case 11:
                $n->field_perro_estado['und'][0]['value'] = 2;
                node_save($n);        
                echo "vial $n->title modificado<br>";
                break;
            case 12:
                $n->field_perro_estado['und'][0]['value'] = 3;
                node_save($n);        
                echo "vial $n->title modificado<br>";
                break;
            case 13:
                $n->field_perro_estado['und'][0]['value'] = 4;
                node_save($n);        
                echo "vial $n->title modificado<br>";
                break;
            case 14:
                $n->field_perro_estado['und'][0]['value'] = 9;
                node_save($n);        
                echo "vial $n->title modificado<br>";
                break;    
               
        }
    }
    /*

1|Pendiente de recepción
2|En análisis
3|Finalizado OK
4|Finalizado No Gen
9|Baja
11|En proceso
12|Finalizado OK
13|Finalizado No Gen
14|Baja
     *      */
     
}
