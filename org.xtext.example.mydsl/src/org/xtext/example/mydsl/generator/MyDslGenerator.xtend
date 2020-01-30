/*
 * generated by Xtext unknown
 */
package org.xtext.example.mydsl.generator

import org.eclipse.emf.common.util.URI
import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.AbstractGenerator
import org.eclipse.xtext.generator.IFileSystemAccess2
import org.eclipse.xtext.generator.IGeneratorContext

/**
 * Generates code from your model files on save.
 * 
 * See https://www.eclipse.org/Xtext/documentation/303_runtime_concepts.html#code-generation
 */
class MyDslGenerator extends AbstractGenerator {

	override void doGenerate(Resource resource, IFileSystemAccess2 fsa, IGeneratorContext context) {
        /*val fileName = URI.decode(resource.URI.trimFileExtension.lastSegment)
        fsa.generateFile(fileName+"Greeter.java", '''
        public class «fileName»Greeter {
            
            public static void main(String[] args) {
                «FOR g : resource.allContents.filter(Greeting).toIterable»
                    System.out.println("Hello «g.name» «IF g.from !== null» from «g.from.name»«ENDIF»!");
                «ENDFOR»
            }
            
        }
        ''')*/
	}
}
