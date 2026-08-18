/*
 * Copyright (C) 2026 The Apache Software Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
package org.opensc.agenda.services.json.plugin;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Annotazione per marcare una classe come plugin.
 * Conservata a runtime per permettere la scansione.
 * Applicabile solo a classi.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface JsonPluginAnnotation
{
  /**
   * Il nome univoco della action.
   * Il valore di default 'auto' viene interpretato come il nome finale della classe.
   * @return
   */
  String nome() default "auto";

  /**
   * Una descrizione estesa da usare in maschera.
   * @return
   */
  String descrizione() default "";

  /**
   * Le action marcate con obsoleto=true vengono ignorati.
   * Il valore di default è false
   * @return
   */
  boolean obsoleto() default false;

  /**
   * Parametri opzionali
   * @return una stringa 'chiave=valore default; chiave=valore default; ...' per eventuali parametri.
   */
  String parametri() default "";

  /**
   * Descrizione parametri opzionali
   * @return una stringa descrizione
   */
  String desparametri() default "";
}
