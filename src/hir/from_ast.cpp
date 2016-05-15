
#include "common.hpp"
#include "hir.hpp"
#include <main_bindings.hpp>
#include <ast/ast.hpp>
#include <ast/crate.hpp>

extern ::HIR::ExprPtr LowerHIR_ExprNode(const ::AST::ExprNode& e);

::HIR::Module LowerHIR_Module(const ::AST::Module& module, ::HIR::SimplePath path);

/// \brief Converts the AST into HIR format
///
/// - Removes all possibility for unexpanded macros
/// - Performs desugaring of for/if-let/while-let/...
::HIR::CratePtr LowerHIR_FromAST(::AST::Crate crate)
{
    ::std::unordered_map< ::std::string, MacroRules >   macros;
    
    // - Extract macros from root module
    for( const auto& mac : crate.m_root_module.macros() ) {
        //if( mac.data.export ) {
        macros.insert( ::std::make_pair( mac.name, mac.data ) );
        //}
    }
    for( const auto& mac : crate.m_root_module.macro_imports_res() ) {
        //if( mac.data->export ) {
        macros.insert( ::std::make_pair( mac.name, *mac.data ) );
        //}
    }
    
    auto rootmod = LowerHIR_Module( crate.m_root_module, ::HIR::SimplePath("") );
    return ::HIR::CratePtr( ::HIR::Crate { mv$(rootmod), mv$(macros) } );
}

// --------------------------------------------------------------------
::HIR::GenericParams LowerHIR_GenericParams(const ::AST::GenericParams& gp)
{
    throw ::std::runtime_error("TODO: LowerHIR_GenericParams");
}

::HIR::ExprPtr LowerHIR_Expr(const ::std::shared_ptr< ::AST::ExprNode>& e)
{
    if( e.get() ) {
        return LowerHIR_ExprNode(*e);
    }
    else {
        return ::HIR::ExprPtr();
    }
}
::HIR::ExprPtr LowerHIR_Expr(const ::AST::Expr& e)
{
    if( e.is_valid() ) {
        return LowerHIR_ExprNode(e.node());
    }
    else {
        return ::HIR::ExprPtr();
    }
}

::HIR::GenericPath LowerHIR_GenericPath(const ::AST::Path& path)
{
    throw ::std::runtime_error("TODO: LowerHIR_GenericPath");
}
::HIR::Path LowerHIR_Path(const ::AST::Path& path)
{
    throw ::std::runtime_error("TODO: LowerHIR_Path");
}

::HIR::TypeRef LowerHIR_Type(const ::TypeRef& ty)
{
    TU_MATCH(::TypeData, (ty.m_data), (e),
    (None,
        TODO(ty.span(), "TypeData::None");
        ),
    (Any,
        return ::HIR::TypeRef();
        ),
    (Unit,
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_Tuple({}) );
        ),
    (Macro,
        BUG(ty.span(), "TypeData::None");
        ),
    (Primitive,
        switch(e.core_type)
        {
        case CORETYPE_BOOL: return ::HIR::TypeRef( ::HIR::CoreType::Bool );
        case CORETYPE_CHAR: return ::HIR::TypeRef( ::HIR::CoreType::Char );
        case CORETYPE_F32:  return ::HIR::TypeRef( ::HIR::CoreType::F32 );
        case CORETYPE_F64:  return ::HIR::TypeRef( ::HIR::CoreType::F64 );
        
        case CORETYPE_I8 :  return ::HIR::TypeRef( ::HIR::CoreType::I8 );
        case CORETYPE_U8 :  return ::HIR::TypeRef( ::HIR::CoreType::U8 );
        case CORETYPE_I16:  return ::HIR::TypeRef( ::HIR::CoreType::I16 );
        case CORETYPE_U16:  return ::HIR::TypeRef( ::HIR::CoreType::U16 );
        case CORETYPE_I32:  return ::HIR::TypeRef( ::HIR::CoreType::I32 );
        case CORETYPE_U32:  return ::HIR::TypeRef( ::HIR::CoreType::U32 );
        case CORETYPE_I64:  return ::HIR::TypeRef( ::HIR::CoreType::I64 );
        case CORETYPE_U64:  return ::HIR::TypeRef( ::HIR::CoreType::U64 );

        case CORETYPE_INT:  return ::HIR::TypeRef( ::HIR::CoreType::Isize );
        case CORETYPE_UINT: return ::HIR::TypeRef( ::HIR::CoreType::Usize );
        case CORETYPE_ANY:
            TODO(ty.span(), "TypeData::Primitive - CORETYPE_ANY");
        case CORETYPE_INVAL:
            BUG(ty.span(), "TypeData::Primitive - CORETYPE_INVAL");
        }
        ),
    (Tuple,
        ::HIR::TypeRef::Data::Data_Tuple v;
        for( const auto& st : e.inner_types )
        {
            v.push_back( LowerHIR_Type(st) );
        }
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_Tuple(mv$(v)) );
        ),
    (Borrow,
        auto cl = (e.is_mut ? ::HIR::BorrowType::Unique : ::HIR::BorrowType::Shared);
        return ::HIR::TypeRef( ::HIR::TypeRef::Data( ::HIR::TypeRef::Data::Data_Borrow { cl, box$(LowerHIR_Type(*e.inner)) } ) );
        ),
    (Pointer,
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_Pointer({e.is_mut, box$(LowerHIR_Type(*e.inner))}) );
        ),
    (Array,
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_Array({
            box$( LowerHIR_Type(*e.inner) ),
            LowerHIR_Expr( e.size )
            }) );
        ),
    
    (Path,
        return ::HIR::TypeRef( LowerHIR_Path(e.path) );
        ),
    (TraitObject,
        if( e.hrls.size() > 0 )
            TODO(ty.span(), "TODO: TraitObjects with HRLS");
        ::HIR::TypeRef::Data::Data_TraitObject  v;
        for(const auto& t : e.traits)
        {
            v.m_traits.push_back( LowerHIR_GenericPath(t) );
        }
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_TraitObject( mv$(v) ) );
        ),
    (Function,
        ::HIR::FunctionType f;
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_Function( mv$(f) ) );
        ),
    (Generic,
        return ::HIR::TypeRef( ::HIR::TypeRef::Data::make_Generic({ e.name, 0 }) );
        )
    )
    throw "BUGCHECK: Reached end of LowerHIR_Type";
}

::HIR::TypeAlias LowerHIR_TypeAlias(const ::AST::TypeAlias& ta)
{
    throw ::std::runtime_error("TODO: LowerHIR_TypeAlias");
}

::HIR::Struct LowerHIR_Struct(const ::AST::Struct& ta)
{
    ::HIR::Struct::Data data;

    throw ::std::runtime_error("TODO: LowerHIR_Struct");
    
    return ::HIR::Struct {
        LowerHIR_GenericParams(ta.params()),
        mv$(data)
        };
}

::HIR::Enum LowerHIR_Enum(const ::AST::Enum& f)
{
    throw ::std::runtime_error("TODO: LowerHIR_Enum");
}
::HIR::Trait LowerHIR_Trait(const ::AST::Trait& f)
{
    throw ::std::runtime_error("TODO: LowerHIR_Trait");
}
::HIR::Function LowerHIR_Function(const ::AST::Function& f)
{
    throw ::std::runtime_error("TODO: LowerHIR_Function");
}

void _add_mod_ns_item(::HIR::Module& mod, ::std::string name, bool is_pub,  ::HIR::TypeItem ti) {
    mod.m_mod_items.insert( ::std::make_pair( mv$(name), ::make_unique_ptr(::HIR::VisEnt< ::HIR::TypeItem> { is_pub, mv$(ti) }) ) );
}
void _add_mod_val_item(::HIR::Module& mod, ::std::string name, bool is_pub,  ::HIR::ValueItem ti) {
    mod.m_value_items.insert( ::std::make_pair( mv$(name), ::make_unique_ptr(::HIR::VisEnt< ::HIR::ValueItem> { is_pub, mv$(ti) }) ) );
}

::HIR::Module LowerHIR_Module(const ::AST::Module& module, ::HIR::SimplePath path)
{
    TRACE_FUNCTION_F("path = " << path);
    ::HIR::Module   mod { };

    for( const auto& item : module.items() )
    {
        auto item_path = path + item.name;
        TU_MATCH(::AST::Item, (item.data), (e),
        (None,
            ),
        (Module,
            _add_mod_ns_item( mod,  item.name, item.is_pub, LowerHIR_Module(e, mv$(item_path)) );
            ),
        (Crate,
            // TODO: All 'extern crate' items should be normalised into a list in the crate root
            // - If public, add a namespace import here referring to the root of the imported crate
            ),
        (Type,
            _add_mod_ns_item( mod,  item.name, item.is_pub, ::HIR::TypeItem::make_TypeAlias( LowerHIR_TypeAlias(e) ) );
            ),
        (Struct,
            /// Add value reference
            TU_IFLET( ::AST::StructData, e.m_data, Struct, e2,
                ::HIR::TypeRef ty = ::HIR::TypeRef( ::HIR::Path(mv$(item_path)) );
                if( e2.ents.size() == 0 )
                    _add_mod_val_item( mod,  item.name, item.is_pub, ::HIR::ValueItem::make_StructConstant({mv$(ty)}) );
                else
                    _add_mod_val_item( mod,  item.name, item.is_pub, ::HIR::ValueItem::make_StructConstructor({mv$(ty)}) );
            )
            _add_mod_ns_item( mod,  item.name, item.is_pub, LowerHIR_Struct(e) );
            ),
        (Enum,
            _add_mod_ns_item( mod,  item.name, item.is_pub, LowerHIR_Enum(e) );
            ),
        (Trait,
            _add_mod_ns_item( mod,  item.name, item.is_pub, LowerHIR_Trait(e) );
            ),
        (Function,
            _add_mod_val_item(mod, item.name, item.is_pub,  LowerHIR_Function(e));
            ),
        (Static,
            if( e.s_class() == ::AST::Static::CONST )
                _add_mod_val_item(mod, item.name, item.is_pub,  ::HIR::ValueItem::make_Constant(::HIR::Constant {
                    ::HIR::GenericParams {},
                    LowerHIR_Type( e.type() ),
                    LowerHIR_Expr( e.value() )
                    }));
            else {
                _add_mod_val_item(mod, item.name, item.is_pub,  ::HIR::ValueItem::make_Static(::HIR::Static {
                    (e.s_class() == ::AST::Static::MUT),
                    LowerHIR_Type( e.type() ),
                    LowerHIR_Expr( e.value() )
                    }));
            }
            )
        )
    }
    
    for( unsigned int i = 0; i < module.anon_mods().size(); i ++ )
    {
        auto& submod = *module.anon_mods()[i];
        ::std::string name = FMT("#" << i);
        auto item_path = path + name;
        _add_mod_ns_item( mod,  name, false, LowerHIR_Module(submod, mv$(item_path)) );
    }

    return mod;
}

