/*
 * generated by Xtext unknown
 */
package org.xtext.example.mydsl

import com.google.inject.Binder
import com.google.inject.name.Names
import org.eclipse.xtext.validation.CompositeEValidator
// import org.eclipse.xtext.validation.*

/**
 * Use this class to register components to be used at runtime / without the Equinox extension registry.
 */
class MyDslRuntimeModule extends AbstractMyDslRuntimeModule {

    def void configureUseEObjectValidator(Binder binder) {
        binder.bind(typeof(Boolean))
        .annotatedWith(
        Names.named(CompositeEValidator.USE_EOBJECT_VALIDATOR))
        .toInstance(Boolean.FALSE);
    }

}
