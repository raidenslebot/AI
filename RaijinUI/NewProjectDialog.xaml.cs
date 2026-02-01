using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

// Raijin ECGI - New Project Creation Dialog
// Intelligent project scaffolding with Raijin assistance

namespace RaijinUI
{
    public partial class NewProjectDialog : Window, INotifyPropertyChanged
    {
        private string _projectName = "";
        private string _projectDescription = "";
        private string _selectedProjectType = "";
        private ProjectTemplate _selectedTemplate;
        private bool _initializeGit = true;
        private bool _createVirtualEnv = false;
        private bool _addDocumentation = true;
        private bool _includeTesting = true;

        public NewProjectDialog()
        {
            InitializeComponent();
            InitializeProjectTypes();
            InitializeTemplates();
            DataContext = this;
        }

        #region Properties

        public string ProjectName
        {
            get => _projectName;
            set { _projectName = value; OnPropertyChanged(); }
        }

        public string ProjectDescription
        {
            get => _projectDescription;
            set { _projectDescription = value; OnPropertyChanged(); }
        }

        public string SelectedProjectType
        {
            get => _selectedProjectType;
            set { _selectedProjectType = value; OnPropertyChanged(); UpdateTemplatesForType(); }
        }

        public ProjectTemplate SelectedTemplate
        {
            get => _selectedTemplate;
            set { _selectedTemplate = value; OnPropertyChanged(); }
        }

        public bool InitializeGit
        {
            get => _initializeGit;
            set { _initializeGit = value; OnPropertyChanged(); }
        }

        public bool CreateVirtualEnv
        {
            get => _createVirtualEnv;
            set { _createVirtualEnv = value; OnPropertyChanged(); }
        }

        public bool AddDocumentation
        {
            get => _addDocumentation;
            set { _addDocumentation = value; OnPropertyChanged(); }
        }

        public bool IncludeTesting
        {
            get => _includeTesting;
            set { _includeTesting = value; OnPropertyChanged(); }
        }

        public ObservableCollection<string> ProjectTypes { get; } = new ObservableCollection<string>();
        public ObservableCollection<ProjectTemplate> ProjectTemplates { get; } = new ObservableCollection<ProjectTemplate>();

        #endregion

        #region Initialization

        private void InitializeProjectTypes()
        {
            ProjectTypes.Add("C/C++ Application");
            ProjectTypes.Add("C# Application");
            ProjectTypes.Add("Python Application");
            ProjectTypes.Add("Web Application");
            ProjectTypes.Add("Machine Learning Project");
            ProjectTypes.Add("Research Project");
            ProjectTypes.Add("System Tool");
            ProjectTypes.Add("Game Development");
            ProjectTypes.Add("Data Analysis");
            ProjectTypes.Add("Custom Project");

            SelectedProjectType = ProjectTypes.First();
        }

        private void InitializeTemplates()
        {
            // Templates will be populated based on selected project type
            UpdateTemplatesForType();
        }

        private void UpdateTemplatesForType()
        {
            ProjectTemplates.Clear();

            switch (SelectedProjectType)
            {
                case "C/C++ Application":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Console Application",
                        Description = "Basic command-line C/C++ application with CMake build system",
                        TemplateType = "cpp_console"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "GUI Application",
                        Description = "Graphical C/C++ application using Qt or GTK",
                        TemplateType = "cpp_gui"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "System Library",
                        Description = "Reusable C/C++ library with proper header organization",
                        TemplateType = "cpp_library"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Performance Critical",
                        Description = "High-performance C/C++ application with optimization focus",
                        TemplateType = "cpp_performance"
                    });
                    break;

                case "C# Application":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Console Application",
                        Description = "Basic .NET console application",
                        TemplateType = "csharp_console"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "WPF Application",
                        Description = "Windows Presentation Foundation GUI application",
                        TemplateType = "csharp_wpf"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "ASP.NET Web API",
                        Description = "RESTful web service using ASP.NET Core",
                        TemplateType = "csharp_webapi"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Class Library",
                        Description = "Reusable .NET class library",
                        TemplateType = "csharp_library"
                    });
                    break;

                case "Python Application":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Console Application",
                        Description = "Basic Python command-line application",
                        TemplateType = "python_console"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Web Application",
                        Description = "Flask/Django web application",
                        TemplateType = "python_web"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Data Science",
                        Description = "Jupyter notebooks and data analysis tools",
                        TemplateType = "python_data"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Machine Learning",
                        Description = "ML project with scikit-learn, TensorFlow, or PyTorch",
                        TemplateType = "python_ml"
                    });
                    break;

                case "Web Application":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "React Application",
                        Description = "Modern React web application with TypeScript",
                        TemplateType = "web_react"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Vue.js Application",
                        Description = "Progressive Vue.js web application",
                        TemplateType = "web_vue"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Angular Application",
                        Description = "Enterprise Angular web application",
                        TemplateType = "web_angular"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Static Site",
                        Description = "Static website with HTML/CSS/JavaScript",
                        TemplateType = "web_static"
                    });
                    break;

                case "Machine Learning Project":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Computer Vision",
                        Description = "Image processing and computer vision project",
                        TemplateType = "ml_vision"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Natural Language Processing",
                        Description = "Text analysis and NLP project",
                        TemplateType = "ml_nlp"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Reinforcement Learning",
                        Description = "RL algorithms and environments",
                        TemplateType = "ml_rl"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "General ML Pipeline",
                        Description = "End-to-end machine learning pipeline",
                        TemplateType = "ml_general"
                    });
                    break;

                case "Research Project":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Academic Research",
                        Description = "Structured research project with documentation",
                        TemplateType = "research_academic"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Experimental Framework",
                        Description = "Scientific experimentation framework",
                        TemplateType = "research_experimental"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Data Collection",
                        Description = "Data acquisition and processing project",
                        TemplateType = "research_data"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Simulation",
                        Description = "Computational simulation project",
                        TemplateType = "research_simulation"
                    });
                    break;

                case "System Tool":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "System Monitor",
                        Description = "System monitoring and diagnostics tool",
                        TemplateType = "system_monitor"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Automation Script",
                        Description = "System automation and scripting tool",
                        TemplateType = "system_automation"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Security Tool",
                        Description = "Security analysis and testing tool",
                        TemplateType = "system_security"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Performance Tool",
                        Description = "System performance analysis tool",
                        TemplateType = "system_performance"
                    });
                    break;

                case "Game Development":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Unity Project",
                        Description = "Unity game development project",
                        TemplateType = "game_unity"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Unreal Engine",
                        Description = "Unreal Engine game development project",
                        TemplateType = "game_unreal"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Custom Engine",
                        Description = "Custom game engine development",
                        TemplateType = "game_custom"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Game Tool",
                        Description = "Game development tool or editor",
                        TemplateType = "game_tool"
                    });
                    break;

                case "Data Analysis":
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Data Pipeline",
                        Description = "ETL and data processing pipeline",
                        TemplateType = "data_pipeline"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Visualization",
                        Description = "Data visualization and dashboard project",
                        TemplateType = "data_visualization"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Analytics Platform",
                        Description = "Business intelligence and analytics platform",
                        TemplateType = "data_analytics"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Database Application",
                        Description = "Database-driven application",
                        TemplateType = "data_database"
                    });
                    break;

                default:
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Blank Project",
                        Description = "Empty project structure for custom development",
                        TemplateType = "custom_blank"
                    });
                    ProjectTemplates.Add(new ProjectTemplate
                    {
                        Name = "Advanced Template",
                        Description = "Advanced project with comprehensive structure",
                        TemplateType = "custom_advanced"
                    });
                    break;
            }

            // Select first template by default
            if (ProjectTemplates.Any())
            {
                SelectedTemplate = ProjectTemplates.First();
            }
        }

        #endregion

        #region Event Handlers

        private void CreateButton_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(ProjectName))
            {
                MessageBox.Show("Please enter a project name.", "Validation Error",
                              MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (SelectedTemplate == null)
            {
                MessageBox.Show("Please select a project template.", "Validation Error",
                              MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            // Validate project name (no special characters that could cause issues)
            if (!IsValidProjectName(ProjectName))
            {
                MessageBox.Show("Project name contains invalid characters. Use only letters, numbers, hyphens, and underscores.",
                              "Invalid Project Name", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            DialogResult = true;
            Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        #endregion

        #region Helper Methods

        private bool IsValidProjectName(string name)
        {
            // Allow letters, numbers, hyphens, underscores, and spaces
            return System.Text.RegularExpressions.Regex.IsMatch(name, @"^[a-zA-Z0-9 _\-]+$");
        }

        #endregion

        #region INotifyPropertyChanged

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion
    }

    #region Data Models

    public class ProjectTemplate
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public string TemplateType { get; set; }
    }

    #endregion
}