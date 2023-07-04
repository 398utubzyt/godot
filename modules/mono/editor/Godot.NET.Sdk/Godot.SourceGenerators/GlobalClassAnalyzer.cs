using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Diagnostics;

namespace Godot.SourceGenerators
{
    [DiagnosticAnalyzer(LanguageNames.CSharp)]
    public class GlobalClassAnalyzer : DiagnosticAnalyzer
    {
        public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics
            => ImmutableArray.Create(
                Common.GlobalClassMustImplementGodotObjectRule,
                Common.GlobalClassMustNotBeGenericRule,
                Common.ParentClassMustBeGlobalRule,
                Common.GlobalClassMustNotBeToolRule);

        public override void Initialize(AnalysisContext context)
        {
            context.ConfigureGeneratedCodeAnalysis(GeneratedCodeAnalysisFlags.None);
            context.EnableConcurrentExecution();
            context.RegisterSyntaxNodeAction(AnalyzeNode, SyntaxKind.ClassDeclaration);
        }

        private void AnalyzeNode(SyntaxNodeAnalysisContext context)
        {
            // Ignore syntax inside comments
            if (IsInsideDocumentation(context.Node))
                return;
            var typeClassDecl = (ClassDeclarationSyntax)context.Node;

            if (context.ContainingSymbol is not INamedTypeSymbol typeSymbol)
                return;

            // Search attributes for [GlobalClass] attribute
            var attrs = typeSymbol?.GetAttributes();
            if (attrs == null)
                return;

            bool isTool = false, isGlobal = false;
            foreach (var attribute in attrs)
            {
                // Could probably break somewhere in here, but
                // realistically nobody is using *that* many attributes

                if (!isGlobal && (attribute.AttributeClass?.IsGodotGlobalClassAttribute() ?? false))
                    isGlobal = true;
                if (!isTool && (attribute.AttributeClass?.IsGodotToolAttribute() ?? false))
                    isTool = true;
            }

            if (!isGlobal)
                return;

            bool isGodot = typeSymbol.InheritsFrom("GodotSharp", GodotClasses.GodotObject);
            if (!isGodot)
                Common.ReportGlobalClassMustImplementGodotObject(context, typeClassDecl, typeSymbol!);

            if (typeSymbol?.IsGenericType ?? false)
                Common.ReportGlobalClassMustNotBeGeneric(context, typeClassDecl, typeSymbol);

            // Global tools have some issues with non-tool resources--intentional?
            // Not really sure how necessary this diagnostic is
            // Older crashes seem to be fixed with https://github.com/godotengine/godot/pull/77377
            if (isTool)
                Common.ReportGlobalClassMustNotBeTool(context, typeClassDecl, typeSymbol!);

            if (!isGodot)
                return;

            // I'd prefer if the GodotSharp stuff was already pre-annotated with [GlobalClass]...
            var baseType = typeSymbol!.BaseType;
            if (!IsObjectFromGodotSharp(baseType) && !(baseType?.GetAttributes()
                .Any(x => x.AttributeClass?.IsGodotGlobalClassAttribute() ?? false) ?? false))
                Common.ReportParentClassMustBeGlobal(context, typeClassDecl, typeSymbol);
        }

        /// <summary>
        /// Check if the type symbol is from the GodotSharp assembly and inherits GodotObject.
        /// </summary>
        /// <param name="type">Type symbol to check.</param>
        /// <returns><see langword="true"/> if the type symbol is a native Godot type. Otherwise, <see langword="false"/>.</returns>
        private bool IsObjectFromGodotSharp(INamedTypeSymbol? type)
            => type?.ContainingAssembly.Name == "GodotSharp" && type.InheritsFrom("GodotSharp", GodotClasses.GodotObject);

        /// <summary>
        /// Check if the syntax node is inside a documentation syntax.
        /// </summary>
        /// <param name="syntax">Syntax node to check.</param>
        /// <returns><see langword="true"/> if the syntax node is inside a documentation syntax.</returns>
        private bool IsInsideDocumentation(SyntaxNode? syntax)
        {
            while (syntax != null)
            {
                if (syntax is DocumentationCommentTriviaSyntax)
                {
                    return true;
                }

                syntax = syntax.Parent;
            }

            return false;
        }
    }
}
