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
                Common.GlobalClassMustDeriveFromGodotObjectRule,
                Common.GlobalClassMustNotBeGenericRule);

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

            // Return if not a type symbol or the type is not a global class.
            if (context.ContainingSymbol is not INamedTypeSymbol typeSymbol ||
                !typeSymbol.GetAttributes().Any(a => a.AttributeClass?.IsGodotGlobalClassAttribute() ?? false))
                return;

            if (typeSymbol.IsGenericType)
                Common.ReportGlobalClassMustNotBeGeneric(context, typeClassDecl, typeSymbol);

            if (!typeSymbol.InheritsFrom("GodotSharp", GodotClasses.GodotObject))
                Common.ReportGlobalClassMustDeriveFromGodotObject(context, typeClassDecl, typeSymbol!);
        }

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
